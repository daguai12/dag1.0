#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <memory>
#include <functional>
#include <mutex>
#include <ucontext.h>

namespace dag{

class Fiber : public std::enable_shared_from_this<Fiber>{
public:
    typedef std::shared_ptr<Fiber> ptr;

    //协程的状态
    enum State {
        READY,
        RUNNING,
        TERM
    };

private:
    // 仅由GetThis()调用 -> 创建主协程
    Fiber();

public:
    Fiber(std::function<void()> cv, size_t stacksize = 0,bool run_in_scheduler = true);

    ~Fiber();

    void reset(std::function<void()> cb);

    void resume();

    void yield();

    uint64_t getId() const {return m_id;}

    State getState() const {return m_state;}

public:
    //设置当前正在运行的协程,即设置线程局部变量t_fiber的值
    static void SetThis(Fiber* f);

    /**
     * @brief 返回当前线程正在执行的协程
     * @details 如果当前线程还未创建协程，则创建协程的第一个协程
     * 且该协程为当前线程的主协程，其他协程都通过这个协程来调度，也就是说，其他协程
     * 结束时，都要切换到主协程，由主协程重新选择新协程进行resume
     * @attention 线程如果要创建协程，那么应该首先执行一下Fiber::GetThis()操作，以初始化主函数协程
    */
    static Fiber::ptr GetThis();

    /**
     * @brief 获取总协程数
    */
    static uint64_t TotalFibers();

    /**
    * @brief 协程入口函数
    */
    static void MainFunc();

    /**
    * @brief 获取当前协程id 
    */
    static uint64_t GetFiberId();

    /**
     * @brief 设置调度协程(默认为主协程)
    */
    static void SetSchedulerFiber(Fiber* f);
private:
    // 协程id 
    uint64_t m_id        = 0;
    // 协程栈大小
    uint32_t m_stacksize = 0;
    // 协程状态 
    State m_state        = READY;
    // 协程上下文 
    ucontext_t m_ctx;
    // 协程栈地址 
    void* m_stack = nullptr;
    // 协程入口函数
    std::function<void()> m_cb;
    // 本协程是否参与调度器调度
    bool m_runInScheduler;
public:
    std::mutex m_mutex;
};

}; 

#endif 
