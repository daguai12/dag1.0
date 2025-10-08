#include "logger.h"
#include "fiber.h"
#include <atomic>
#include <iostream>

#include <assert.h>

namespace dag{
static Logger::ptr g_logger = DAG_LOG_ROOT();

// 正在运行的协程
static thread_local Fiber* t_fiber = nullptr;
// 主协程
static thread_local std::shared_ptr<Fiber> t_thread_fiber = nullptr;
// 调度协程
static thread_local Fiber* t_scheduler_fiber = nullptr; 

// 协程id
static std::atomic<uint64_t> s_fiber_id{0};
// 协程计数器
static std::atomic<uint64_t> s_fiber_count{0};  


/**
 * @brief 返回当前线程正在执行的协程
 * @details 如果当前线程还未创建协程，则创建线程的低一个协程,
 * 且该协程为当前线程的主协程，其他协程都通过这个协程来调度，也就是说，其他协程
 * 结束时，都要切换回主协程，由主协程重新选择新的协程进行resume
 * @attention 线程如果要创建协程，那么应该首先执行一下Fiber::GetThis()操作，以初始化主函数协程
 */
std::shared_ptr<Fiber> Fiber::GetThis()
{
    if(t_fiber)
    {
        return t_fiber->shared_from_this();
    }

    std::shared_ptr<Fiber> main_fiber(new Fiber());
    t_thread_fiber = main_fiber;
    t_scheduler_fiber = main_fiber.get(); 

    assert(t_fiber == main_fiber.get());
    return t_fiber->shared_from_this();
}


/**
 * @brief 设置协程函数
 */
void Fiber::SetThis(Fiber* f)
{
    t_fiber = f;
}


/**
 * @brief 获取协程ID
 */
uint64_t Fiber::GetFiberId()
{
    if(t_fiber)
    {
        return t_fiber->getId();
    }
    return (uint64_t)-1;
}


/**
* @brief 构造函数
* @attention 无参构造函数只用于创建线程的第一个协程，也就是线程主函数对应的协程，
* 这个协程只能由GetThis()方法调用，所以定义成私有方法
*/
Fiber::Fiber() {
    SetThis(this);
    m_state = RUNNING;

    if (getcontext(&m_ctx)) {
        std::cerr << "getcontext error" << std::endl;
    }

    ++s_fiber_count;
    m_id = s_fiber_id++;  //协程id从0开始，用完加1
    #if DEBUG
    DAG_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
    #endif
}


/**
* @brief 构造函数，用于创建用户协程
* @param[] cb 协程入口函数
* @param[] stacksize 栈大小，默认为128k
*/
Fiber::Fiber(std::function<void()> cb, size_t stacksize,bool run_in_scheduler)
    : m_cb(cb)
    , m_runInScheduler(run_in_scheduler)
{
    m_state = READY;

    //分配协程栈空间
    m_stacksize = stacksize ? stacksize : 128000;
    m_stack = malloc(m_stacksize);

    if(getcontext(&m_ctx))
    {
        std::cerr << "Fiber(std::function<void> cb, size_t stacksize, bool run_in_scheduler) failed\n";
        pthread_exit(NULL);
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx,&Fiber::MainFunc,0);

    m_id = s_fiber_id++;
    ++s_fiber_count;
    #if DEBUG
    DAG_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
    #endif
}

/**
* @brief 析构函数，线程结束时，销毁用户创建的协程的空间
*/
Fiber::~Fiber()
{
    --s_fiber_count;
    if(m_stack)
    {
        free(m_stack);
    }
    #if DEBUG
    DAG_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                            << " total=" << s_fiber_count;
    #endif
}

/**
* @brief 重复利用已结束的协程，复用栈空间，创建新协程
*/
void Fiber::reset(std::function<void()> cb)
{
    assert(m_stack != nullptr && m_state == TERM);

    m_state = READY;
    m_cb = cb;

    if(getcontext(&m_ctx))
    {
        std::cerr << "reset() failed\n";
        pthread_exit(NULL);
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

/**
* @brief 将当前协程切换到执行状态
* @details 当前协程和正在运行的协程进行交换，前者变为RUNNING,后者变为READY
*/
void Fiber::resume()
{
    assert(m_state==READY);

    m_state = RUNNING;

    if(m_runInScheduler)
    {
        SetThis(this);
        if(swapcontext(&(t_scheduler_fiber->m_ctx), &m_ctx))
        {
            std::cerr << "resume() to t_scheduler_fiber failed\n";
            pthread_exit(NULL);
        }
    }
    else
    {
        SetThis(this);
        if(swapcontext(&(t_thread_fiber->m_ctx), &m_ctx))
        {
            std::cerr << "resume to t_thread_fiber failed\n";
            pthread_exit(NULL);
        }
    }
}

/**
* @brief 当前协程让出执行权
* @details 当前协程与上次resume时退出后台的协程进行交换，前者状态变为READY,后者变为RUNNING
*/ 
void Fiber::yield()
{
    assert(m_state==RUNNING || m_state==TERM);

    if(m_state!=TERM)
    {
        m_state = READY;
    }

    if(m_runInScheduler)
    {
        SetThis(t_scheduler_fiber);
        if(swapcontext(&m_ctx, &(t_scheduler_fiber->m_ctx)))
        {
            std::cerr << "yield() to t_scheduler_fiber failed\n";
            pthread_exit(NULL);
        }
    }
    else
    {
        SetThis(t_thread_fiber.get());
        if(swapcontext(&m_ctx, &(t_thread_fiber->m_ctx)))
        {
            std::cerr << "yield() to t_thread_fiber failed\n";
            pthread_exit(NULL);
        }
    }
}

/**
* @brief 协程入口函数
* @note 这里没有处理协程函数出现异常情况，同样是为了简化状态管理，并且个人认为协程的异常不应该由框架
* 处理，应该由开发者自行处理
*/
void Fiber::MainFunc()
{
    std::shared_ptr<Fiber> curr = GetThis();
    assert(curr!=nullptr);

    curr->m_cb();
    curr->m_cb = nullptr;
    curr->m_state = TERM;

    //运行完毕 -> 让出执行权力
    auto raw_ptr = curr.get();
    curr.reset();
    raw_ptr->yield();
}

/**
* @brief 设置调度协程(默认为主协程)
*/
void Fiber::SetSchedulerFiber(Fiber* f)
{
    t_scheduler_fiber = f;
}

};
