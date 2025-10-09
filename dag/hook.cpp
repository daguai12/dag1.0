#include <iostream>
#include <cerrno>
#include <dlfcn.h>
#include <cstdarg>
#include <string.h>


#include "hook.h"
#include "ioscheduler.h"
#include "fd_manager.h"
#include "logger.h"

// apply XX to all functions
#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(getsockopt) \
    XX(setsockopt) 

namespace dag{

// if this thread is using hooked function 
static thread_local bool t_hook_enable = false;

bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

void hook_init()
{
	static bool is_inited = false;
	if(is_inited)
	{
		return;
	}
    #if DEBUG
	is_inited = true;
    #endif
    // assignment -> sleep_f = (sleep_fun)dlsym(RTLD_NEXT, "sleep"); 保存c标准库中函数的地址
    #define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX)
    #undef XX
}

// 静态变量在main函数执行前初始化完成
struct HookIniter
{
	HookIniter()
	{
		hook_init();
	}
};

static HookIniter s_hook_initer;

} // end namespace dag

struct timer_info 
{
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name, uint32_t event, int timeout_so, Args&&... args) 
{
    if(!dag::t_hook_enable)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(fd);
    if(!ctx) 
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClosed()) 
    {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) 
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取超时时间
    uint64_t timeout = ctx->getTimeout(timeout_so);
    // 设置条件
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    
    // 被信号中断重新启动系统调用
    while(n == -1 && errno == EINTR)
    {
        n = fun(fd, std::forward<Args>(args)...);
    }
    
    // 没有数据可以读取
    if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        dag::IOManager* iom = dag::IOManager::GetThis();
        // 定时器指针
        std::shared_ptr<dag::Timer> timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        // 1. 如果设置了超时时间 -> 添加条件定时器，用于在超时后取消该操作
        if(timeout != (uint64_t)-1) 
        {
            timer = iom->addConditionTimer(timeout, [winfo, fd, iom, event]()
            {
                auto t = winfo.lock();
                if(!t || t->cancelled) 
                {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                // 取消该事件，并触发一次以唤醒等待的协程
                iom->cancelEvent(fd, (dag::IOManager::Event)(event));
            }, winfo);
        }

        // 2 向IOManager注册事件 -> 回调函数为当前协程
        int rt = iom->addEvent(fd, (dag::IOManager::Event)(event));
        if(rt)
        {
            std::cout << hook_fun_name << " addEvent("<< fd << ", " << event << ")";
            if(timer) 
            {
                timer->cancel();
            }
            return -1;
        } 
        else 
        {
            // 当前协程让出执行权，等待事件或定时器唤醒
            dag::Fiber::GetThis()->yield();
     
            // 3 被addEvent或cancelEvent唤醒后执行
            if(timer) 
            {
                timer->cancel();
            }
            // 如果是因为定时器超时导致的取消事件
            if(tinfo->cancelled == ETIMEDOUT) 
            {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}


#ifdef __cplusplus
extern "C"{
#endif

// 声明函数指针 -> sleep_fun sleep_f = nullptr;
#define XX(name) name ## _fun name ## _f = nullptr;
	HOOK_FUN(XX)
#undef XX

unsigned int sleep(unsigned int seconds)
{
	if(!dag::t_hook_enable)
	{
		return sleep_f(seconds);
	}

	std::shared_ptr<dag::Fiber> fiber = dag::Fiber::GetThis();
	dag::IOManager* iom = dag::IOManager::GetThis();
	// add a timer to reschedule this fiber
	iom->addTimer(seconds*1000, [fiber, iom](){iom->schedulerLock(fiber, -1);});
	// wait for the next resume
	fiber->yield();
	return 0;
}

int usleep(useconds_t usec)
{
	if(!dag::t_hook_enable)
	{
		return usleep_f(usec);
	}

	std::shared_ptr<dag::Fiber> fiber = dag::Fiber::GetThis();
	dag::IOManager* iom = dag::IOManager::GetThis();
	iom->addTimer(usec/1000, [fiber, iom](){iom->schedulerLock(fiber);});
	fiber->yield();
	return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem)
{
	if(!dag::t_hook_enable)
	{
		return nanosleep_f(req, rem);
	}	

	int timeout_ms = req->tv_sec*1000 + req->tv_nsec/1000/1000;

	std::shared_ptr<dag::Fiber> fiber = dag::Fiber::GetThis();
	dag::IOManager* iom = dag::IOManager::GetThis();
	iom->addTimer(timeout_ms, [fiber, iom](){iom->schedulerLock(fiber, -1);});
	fiber->yield();	
	return 0;
}

int socket(int domain, int type, int protocol) noexcept
{
	if(!dag::t_hook_enable)
	{
		return socket_f(domain, type, protocol);
	}	

	int fd = socket_f(domain, type, protocol);
	if(fd==-1)
	{
		std::cerr << "socket() failed:" << strerror(errno) << std::endl;
		return fd;
	}
	dag::FdMgr::GetInstance()->get(fd, true);
	return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) 
{
    if(!dag::t_hook_enable) 
    {
        return connect_f(fd, addr, addrlen);
    }

    std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isClosed()) 
    {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) 
    {
        return connect_f(fd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) 
    {

        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) 
    {
        return 0;
    } 
    else if(n != -1 || errno != EINPROGRESS) 
    {
        return n;
    }

    dag::IOManager* iom = dag::IOManager::GetThis();
    std::shared_ptr<dag::Timer> timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) 
    {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() 
        {
            auto t = winfo.lock();
            if(!t || t->cancelled) 
            {
                return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(fd, dag::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(fd, dag::IOManager::WRITE);
    if(rt == 0) 
    {
        dag::Fiber::GetThis()->yield();

        if(timer) 
        {
            timer->cancel();
        }

        if(tinfo->cancelled) 
        {
            errno = tinfo->cancelled;
            return -1;
        }
    } 
    else 
    {
        if(timer) 
        {
            timer->cancel();
        }
        std::cerr << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) 
    {
        return -1;
    }
    if(!error) 
    {
        return 0;
    } 
    else 
    {
        errno = error;
        return -1;
    }
}


static uint64_t s_connect_timeout = -1;
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return connect_with_timeout(sockfd, addr, addrlen, s_connect_timeout);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int fd = do_io(sockfd, accept_f, "accept", dag::IOManager::READ, SO_RCVTIMEO, addr, addrlen);	
	if(fd>=0)
	{
		dag::FdMgr::GetInstance()->get(fd, true);
	}
	return fd;
}

ssize_t read(int fd, void *buf, size_t count)
{
	return do_io(fd, read_f, "read", dag::IOManager::READ, SO_RCVTIMEO, buf, count);	
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
	return do_io(fd, readv_f, "readv", dag::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);	
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
	return do_io(sockfd, recv_f, "recv", dag::IOManager::READ, SO_RCVTIMEO, buf, len, flags);	
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	return do_io(sockfd, recvfrom_f, "recvfrom", dag::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);	
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	return do_io(sockfd, recvmsg_f, "recvmsg", dag::IOManager::READ, SO_RCVTIMEO, msg, flags);	
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return do_io(fd, write_f, "write", dag::IOManager::WRITE, SO_SNDTIMEO, buf, count);	
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
	return do_io(fd, writev_f, "writev", dag::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);	
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	return do_io(sockfd, send_f, "send", dag::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);	
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	return do_io(sockfd, sendto_f, "sendto", dag::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags, dest_addr, addrlen);	
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
	return do_io(sockfd, sendmsg_f, "sendmsg", dag::IOManager::WRITE, SO_SNDTIMEO, msg, flags);	
}

int close(int fd)
{
	if(!dag::t_hook_enable)
	{
		return close_f(fd);
	}	

	std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(fd);

	if(ctx)
	{
		auto iom = dag::IOManager::GetThis();
		if(iom)
		{
			iom->cancelAll(fd);
		}
		dag::FdMgr::GetInstance()->del(fd);
	}
	return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ )
{
    va_list va; 

    va_start(va, cmd);
    switch(cmd) 
    {
        case F_SETFL:
            {
                int arg = va_arg(va, int); // Access the next int argument
                va_end(va);
                std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) 
                {
                    return fcntl_f(fd, cmd, arg);
                }
                // 用户是否设定了非阻塞
                ctx->setUserNonblock(arg & O_NONBLOCK);
                // 最后是否阻塞根据系统设置决定
                if(ctx->getSysNonblock()) 
                {
                    arg |= O_NONBLOCK;
                } 
                else 
                {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;

        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) 
                {
                    return arg;
                }
                // 这里是呈现给用户 显示的为用户设定的值
                // 但是底层还是根据系统设置决定的
                if(ctx->getUserNonblock()) 
                {
                    return arg | O_NONBLOCK;
                } else 
                {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;

        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;


        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;

        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;

        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;

        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }	
}


int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) noexcept
{
	return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) noexcept
{
    if(!dag::t_hook_enable) 
    {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }

    if(level == SOL_SOCKET) 
    {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) 
        {
            std::shared_ptr<dag::FdCtx> ctx = dag::FdMgr::GetInstance()->get(sockfd);
            if(ctx) 
            {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);	
}

#ifdef __cplusplus
}
#endif
