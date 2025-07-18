#ifndef _IO_MANAGE_H_
#define _IO_MANAGE_H_

#include "scheduler.h"
#include "timer.h"

namespace dag 
{

class IOManager : public Scheduler, public TimerManager {
public:
    /**
    * @brief IO事件,继承自epoll对事件的定义
    * @details 这里只关心socket fd的读和写事件，其他epoll事件都归类到这两个事件中
    */
    enum Event
    {
        NONE = 0x0,
        // READ == EPOLLIN
        READ = 0x1,
        // WRITE == EPOLLOUT
        WRITE = 0x4
    };

private:
    /**
    * @brief 事件的上下文类
    * @details fd的每个事件都有一个事件上下文，保存这个事件的回调函数以及执行回调函数的调度器
    */
    struct FdContext
    {
        struct EventContext
        {
            // 执行事件回调的调度器
            Scheduler *scheduler = nullptr;
            // 事件回调协程
            Fiber::ptr fiber;
            // 事件回调函数
            std::function<void()> cb;
        };

        // 读事件上下文
        EventContext read;
        // 写事件上下文
        EventContext write;
        // 事件关联的句柄
        int fd = 0;
        // 该fd添加了那些事件的回掉函数,或者说该fd关心那些事件
        Event events = NONE;
        // 事件的mutex
        std::mutex mutex;

        /**
        * @brief 获取事件的上下文
        * @param[in] event事件类型
        * @return 返回对应的事件的上下文
        */
        EventContext &getEventContext(Event event);

        /**
        * @brief 重置事件上下文
        * @param[in,out] ctx 待重置的事件上下文对象
        */
        void resetEventContext(EventContext &event);

        /**
        * @brief 触发事件
        * @details 根据事件类型调用对应上下文结构中的调度器去调度回调协程或回调函数
        * @param[in] event 事件类型
        */
        void triggerEvent(Event event);

    };

public:
    /**
    * @brief 构造函数
    * @param[in] threads 线程数量
    * @param[in] name 调度器的名称
    */
    IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "IOManager");

    ~IOManager();

    /**
     * @brief 添加事件
     * @details fd描述符发生了event事件时执行cb函数
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @param[in] cb事件回调函数，如果为空，则默认把当前协程作为回调执行体
     * @return 添加成功返回0,失败返回 -1
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

    /**
     * @brief 删除事件
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @attention 不会触发事件
     * @return 是否删除成功
     */
    bool delEvent(int fd, Event event);

    // delete the event and trigger its callback
    bool cancelEvent(int fd, Event event);


    /**
     * @brief 取消所有事件
     * @details 所有被注册的回调事件在cancel之前都会被执行一次
     * @param[in] fd socket句柄
     * @return 是否删除成功
     */
    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    /**
    * @brief 通知调度器有任务要调度
    * @details 写pipe让idle协程从epoll_wait退出，待idle协程yield之后Scheduler::run可以调度其他任务
    *          如果当前没有空闲调度线程，就没有必要发通知
    */
    void tickle() override;
    
    bool stopping() override;
    
    /**
    * @brief idle协程
    * @details 对于IO协程调度来说，应阻塞在等待IO事件上，idle推出的时机是epoll_wait返回，对应的操作是
    * tickle或注册的IO事件就绪
    * 调度器无调度任务时会阻塞idle协程上，对IO调度器而言，idle状态应该关注两个事件，
    * 一是没有新的调度任务，对应Scheduler::scheduler(),如果有新的调度任务，那应该立即退出idle状态，并执行对于的任务;
    * 二是关注当前注册的所有IO事件有没有触发，如果有触发，应该立即执行IO事件对应的回调函数
    */
    void idle() override;

    void onTimerInsertedAtFront() override;

    void contextResize(size_t size);
    
private:
    // epoll 文件句柄
    int m_epfd = 0;
    // pipe 文件句柄，fd[0]读端，fd[1]写端
    int m_tickleFds[2];
    // 当前等待执行的IO事件数量
    std::atomic<size_t> m_pendingEvenCount = {0};
    // IOManager的读写锁
    std::shared_mutex m_mutex;
    // socket事件上下文的容器
    std::vector<FdContext*> m_fdContexts;
};

}

#endif

