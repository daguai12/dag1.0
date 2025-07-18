#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
    
#include <functional>
#include <vector>
#include <atomic>

#include "fiber.h"
#include "thread.h"


namespace dag{

/**
 * @brief 协程调度器
 * @details 封装N-M的协程调度器
 *          内部封装一个线程池，支持协程在线程池里切换
*/

class Scheduler
{
public:

    /**
     * @brief 创建调度器
     * @param[in] threads 线程数
     * @param[in] use_caller 是否将当前线程也作为调度线程
     * @param[in] name 名称
    */
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "Scheduler");

    /**
     * @brief 析构函数
    */
    virtual ~Scheduler();

    /**
     * @brief 获取调度器名称
    */
    const std::string& getName() const {return m_name;}

public:
    /**
     * @brief 获取当前线程调度器的指针
    */
    static Scheduler* GetThis();

protected:
    /**
     * @brief设置当前的协程调度器
    */
    void SetThis();

public:
    /**
    * @brief 添加调度任务
    * @tparam FiberOrCb 调度任务类型，可以是协程对象或函数指针
    * @param[] fc协程对象或指针
    * @param[] thread 指定运行该任务的线程号，-1表示任意线程
    */
    template <class FiberOrCb>
    void schedulerLock(FiberOrCb fc, int thread = -1)
    {
        bool need_tickle;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // empty -> all thread is idle -> need to be waken up
            need_tickle = m_tasks.empty();

            SchedulerTask task(fc,thread);
            if (task.fiber || task.cb)
            {
                m_tasks.push_back(task);
            }
        }

        if(need_tickle)
        {
            tickle(); //唤醒idle协程
        }
    }

    /**
     * @brief 启动线程池
    */
    virtual void start();

    /**
     * @brief 关闭线程池
    */
    virtual void stop();

protected:
    /**
     * @brief 通知协程调度器有任务了
    */
    virtual void tickle();

    /**
    * @brief 协程调度函数
    */
    virtual void run();


    /**
    * @brief 无任务调度时执行idle协程
    */
    virtual void idle();

    /**
    * @brief 返回是否可以停止
    */
    virtual bool stopping();

    /**
    * @brief 返回是否有空闲线程
    * @details 当调度协程进入idle时空闲线程加1,从idle协程返回时空闲线程数减1
    */
    bool hasIdleThreads() {return m_idleThreadCount > 0;}

private:
    /**
     * @brief 调度任务，协程/函数二选一，可指定在那个线程上调度
    */
    struct SchedulerTask
    {
        std::shared_ptr<Fiber> fiber;
        std::function<void()> cb;
        int thread; //指定任务需要运行的线程id

        SchedulerTask()
        {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }

        SchedulerTask(std::shared_ptr<Fiber> f,int thr)
        {
            fiber = f;
            thread = thr;
        }

        SchedulerTask(std::shared_ptr<Fiber>* f,int thr)
        {
            fiber.swap(*f);
            thread = thr;
        }

        SchedulerTask(std::function<void()> f,int thr)
        {
            cb = f;
            thread = thr;
        }

        SchedulerTask(std::function<void()>* f,int thr)
        {
            cb.swap(*f);
            thread = thr;
        }

        void reset()
        {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    // 协程调度器名称
    std::string m_name;
    // 互斥锁
    std::mutex m_mutex;
    // 线程池
    std::vector<std::shared_ptr<Thread>> m_threads;
    // 任务队列
    std::vector<SchedulerTask> m_tasks;
    // 存储工作线程的线程id
    std::vector<int> m_threadIds;
    // 需要额外创建的线程数
    size_t m_threadCount = 0;
    // 活跃线程数
    std::atomic<size_t> m_activeThreadCount = {0};
    // 空闲线程数
    std::atomic<size_t> m_idleThreadCount = {0};

    // 主线程是否用工作线程
    bool m_useCaller;
    // 如果是 -> 需要额外创建调度协程
    std::shared_ptr<Fiber> m_schedulerFiber;
    // 如果是 -> 记录主线程的线程id
    int m_rootThread = -1;
    // 是否正在关闭
    bool m_stopping = false;
};

};

#endif
