#include <sys/epoll.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

#include "ioscheduler.h"
#include "scheduler.h"
#include "./utils/util.h"
#include "logger.h"
#include "./utils/asserts.h"


// int debug = 0;

namespace dag
{

static dag::Logger::ptr g_logger = DAG_LOG_ROOT();

IOManager* IOManager::GetThis()
{
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

IOManager::FdContext::EventContext& IOManager::FdContext::getEventContext(Event event)
{
    assert(event==READ || event ==WRITE);
    switch(event)
    {
    case READ:
        return read;
    case WRITE:
        return write;
    default:
        DAG_ASSERT(false);
    }
    throw std::invalid_argument("Unsupported event type");
}


IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
    : Scheduler(threads, use_caller, name)
{
    //create epoll fd
    m_epfd = epoll_create(5000);
    assert(m_epfd > 0);

    //craete pipe
    int rt = pipe(m_tickleFds);
    assert(!rt);

    // add read event to epoll
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET; // ET模式
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);
    assert(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    assert(!rt);

    contextResize(32);

    // 开启Scheduler, IOManager创建可以调度的协程
    start();
}

void IOManager::FdContext::resetEventContext(EventContext &ctx)
{
    ctx.scheduler = nullptr;
    ctx.cb = nullptr;
    ctx.fiber.reset();
}

void IOManager::FdContext::triggerEvent(IOManager::Event event)
{
    assert(events & event);

    events = (Event)(events & ~event);

    EventContext& ctx = getEventContext(event);

    if (ctx.cb)
    {
        ctx.scheduler->schedulerLock(&ctx.cb);
    }
    else
    {
        ctx.scheduler->schedulerLock(&ctx.fiber);
    }

    resetEventContext(ctx);
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
{
    FdContext* fd_ctx = nullptr;

    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if ((int)m_fdContexts.size() > fd)
    {
        fd_ctx = m_fdContexts[fd];
        read_lock.unlock();
    }
    else
    {
        read_lock.unlock();
        std::unique_lock<std::shared_mutex> write_lock(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    std::lock_guard<std::mutex> lock(fd_ctx->mutex);

    // the event has already been added
    // if (fd_ctx->events & event)
    // {
    //     return -1;
    // }
    
     if(fd_ctx->events & event) {
        #if DEBUG
        DAG_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                    << " event=" << (EPOLL_EVENTS)event
                    << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
        #endif
        DAG_ASSERT(!(fd_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events =  EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);

    if (rt) {
        #if DEBUGJ
        DAG_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        #endif
        return -1;
    }

    ++m_pendingEvenCount;
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getEventContext(event);
    assert(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);

    event_ctx.scheduler = Scheduler::GetThis();
    if(cb)
    {
        event_ctx.cb.swap(cb);
    }
    else
    {
        event_ctx.fiber = Fiber::GetThis();
        assert(event_ctx.fiber->getState() == Fiber::RUNNING);
    }

    return 0;
}

bool IOManager::delEvent(int fd, Event event)
{
    FdContext* fd_ctx = nullptr;
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if ((int)m_fdContexts.size() > 0)
    {
        fd_ctx = m_fdContexts[fd];
        read_lock.unlock();
    }
    else
    {
        read_lock.unlock();
        return false;
    }

    std::lock_guard<std::mutex> lock(fd_ctx->mutex);
    if (!(fd_ctx->events & event))
    {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op =  new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt)
    {
        std::cerr << "delEvent::epoll_ctl faield: " << strerror(errno) << std::endl;
    }
    --m_pendingEvenCount;

    fd_ctx->events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getEventContext(event);
    fd_ctx->resetEventContext(event_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd,Event event) 
{
    FdContext *fd_ctx = nullptr;

    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if ((int)m_fdContexts.size() > 0)
    {
        fd_ctx = m_fdContexts[fd];
        read_lock.unlock();
    }
    else
    {
        read_lock.unlock();
        return false;
    }

    std::lock_guard<std::mutex> lock(fd_ctx->mutex);
    if (!(fd_ctx->events & event))
    {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = new_events | EPOLLET;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    // if (rt)
    // {
    //     std::cerr << "cancelEvent::epoll_ctl failed: " << strerror(errno) << std::endl;
    // }

    if (rt) {
        #if DEBUGJ
        DAG_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        #endif
        return false;
    }
    --m_pendingEvenCount;
    fd_ctx->triggerEvent(event);
    return true;
}

bool IOManager::cancelAll(int fd) 
{
    FdContext* fd_ctx = nullptr;
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if ((int)m_fdContexts.size() > 0)
    {
        fd_ctx = m_fdContexts[fd];
        read_lock.unlock();
    }
    else
    {
        read_lock.unlock();
        return false;
    }

    std::lock_guard<std::mutex> lock(fd_ctx->mutex);
    if (!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    // if (rt) {
    //     std::cerr << "IOManager::epoll_ctl failed: " << strerror(errno) << std::endl;
    //     return -1;
    // }

    if (rt) {
        #if DEBUG
        DAG_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        #endif
        return false;
    }
    
    if (fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEvenCount;
    }

    if (fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEvenCount;
    }

    assert(fd_ctx->events == 0);
    return true;
}

void IOManager::tickle() {
    // no idle threads
    if (!hasIdleThreads())
    {
        return;
    }
    int rt = write(m_tickleFds[1],"T",1);
    assert(rt == 1);
}

bool IOManager::stopping()
{
    uint64_t timeout = getNextTimer();
    return timeout == ~0ull && m_pendingEvenCount == 0 &&Scheduler::stopping();
}

void IOManager::idle() 
{
    // 一次epoll_wait最多检测到256个就绪事件，如果就绪事件超过了这个数，那么会在下轮epoll_wait继续处理
    #if DEBUG
    DAG_LOG_DEBUG(g_logger) << "idle";
    #endif
    static const uint64_t MAX_EVENTS = 256;
    std::unique_ptr<epoll_event[],void(*)(epoll_event*)> events(new epoll_event[MAX_EVENTS],[](epoll_event* ep) {
        delete[] ep;
    });
    
    while (true)
    {
        // if(debug) std::cout << "IOManager::idle(),run in thread: " << getThreadId() << std::endl;

        if(stopping())
        {
            // if(debug) std::cout << "name = " << getName() << " idle exits in thread: " << getThreadId() << std::endl;
            break;
        }

        //blocked at epoll_wait
        int rt = 0;
        while(true)
        {
            static const uint64_t MAX_TIMEOUT = 5000;
            uint64_t next_timeout = getNextTimer(); //最近要到期的定时器
            next_timeout = std::min(next_timeout,MAX_TIMEOUT); //限制最大等待时间
            rt = epoll_wait(m_epfd,events.get(),MAX_EVENTS,(int)next_timeout);
            if (rt < 0 && errno == EINTR)
            {
                continue;
            }
            else
            {
                break;
            }
        };

            std::vector<std::function<void()>> cbs;
            listExpiredCb(cbs);
            
            if(!cbs.empty())
            {
                for(const auto& cb : cbs)
                {
                    schedulerLock(cb);
                }
                cbs.clear();
            }

            for (int i = 0;i < rt;++i)
            {
                epoll_event& event = events[i];

                if (event.data.fd == m_tickleFds[0])
                {
                    uint8_t dummy[256];

                    while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);
                    continue;
                }

                FdContext* fd_ctx = (FdContext *)event.data.ptr;
                std::lock_guard<std::mutex> lock(fd_ctx->mutex);

                if (event.events & (EPOLLERR | EPOLLHUP))
                {
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
                }

                int real_events = NONE;
                if (event.events & EPOLLIN)
                {
                    real_events |= READ;
                }
                if (event.events & EPOLLOUT)
                {
                    real_events |= WRITE;
                }
                if ((fd_ctx->events & real_events) == NONE)
                {
                    continue;
                }

                int left_events = (fd_ctx->events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
                // if (rt2)
                // {
                //     std::cerr << "idle::epoll_ctl failed: " << strerror(errno) << std::endl;
                //     continue;
                // }
                if(rt2) {
                    #if DEBUGJ
                    DAG_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                        << op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                        << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                    #endif
                    continue;
                }

                if (real_events & READ)
                {
                    fd_ctx->triggerEvent(READ);
                    --m_pendingEvenCount;
                }
                if (real_events & WRITE)
                {
                    fd_ctx->triggerEvent(WRITE);
                    --m_pendingEvenCount;
                }
            } //end for
            Fiber::GetThis()->yield();
    } // end while(true)
}

IOManager::~IOManager()
{
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for (size_t i = 0;i < m_fdContexts.size(); ++i) {
        if (m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i)
    {
        if (m_fdContexts[i] == nullptr)
        {
            m_fdContexts[i] = new FdContext();
            m_fdContexts[i]->fd = i;
        }
    }
}

void IOManager::onTimerInsertedAtFront()
{
    tickle();
}

}

