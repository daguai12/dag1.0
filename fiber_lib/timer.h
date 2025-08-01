#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <memory>
#include <vector>
#include <set>
#include <shared_mutex>
#include <functional>

namespace dag
{

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;
public:
    /**
    * @brief 从时间堆中删除timer
    */
    bool cancel();

    /**
    * @brief 刷新timer
    */
    bool refresh();

    /**
    * @brief重设timer的超时时间
    * @param[in] ms 定时器执行间隔时间(毫秒)
    * @param[in] from_now 是否从当前时间开始计算
    */
    bool reset(uint64_t ms, bool from_now);

private:
    /**
     * @brief 构造函数
     * @param[in] ms 定时器执行间隔时间
     * @param[in] cb 回调函数
     * @param[in] recurring 是否循环
     * @param[in] manager 定时器管理器
     */
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);

private:
    // 是否循环
    bool m_recurring = false;
    // 超时时间
    uint64_t m_ms = 0;
    // 绝对超时时间
    std::chrono::time_point<std::chrono::system_clock> m_next;
    // 超时时触发的回掉函数
    std::function<void()> m_cb;
    // 管理此timer的管理器
    TimerManager* m_manager = nullptr;

private:
    /**
     * @brief实现最小堆的比较函数
     */
    struct Comparator
    {
        /**
         * @brief 比较定时器的智能指针的大小(按执行时间排序)
         * @param[in] lsh 定时器智能指针
         * @param[in] rhs 定时器智能指针
         */
        bool operator()(const std::shared_ptr<Timer>& lhs, const std::shared_ptr<Timer>& rhs) const;
    };
};


/**
 * @brief 定时器管理器
 */

class TimerManager
{
    friend class Timer;
public:
    /**
     * @brief 构造函数
     */
    TimerManager();
    
    /**
     * @brief 析构函数
     */
    virtual ~TimerManager();

    /**
    * @brief添加timer
    * @param[in] ms 定时器执行间隔时间
    * @param[in] cb 定时器回调函数
    * @param[in] recurring 是否循环定时器
    */
    std::shared_ptr<Timer> addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

    /**
    * @brief 添加条件timer
    * @param[in] ms 定时器执行间隔时间
    * @param[in] cb 定时器回调函数
    * @param[in] weak_cond条件
    * @param[in] recurring 是否循环
    */
    std::shared_ptr<Timer> addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond,bool recurring = false);
    
    /**
    * @brief 拿到堆中最近的超时时间
    */
    uint64_t getNextTimer();

    /**
    * @brief 取出所有超时定时器的回调函数
    * @param[out] cbs 回调函数数组
    */
    void listExpiredCb(std::vector<std::function<void()>>& cbs);

    //堆中是否有timer
    bool hasTimer();

protected:
    /**
    * @brief 当有新的定时器插入到定时器的首部，执行该函数
    */
    virtual void onTimerInsertedAtFront() {}

    /**
    * @brief 将定时器添加到管理器中
    */
    void addTimer(std::shared_ptr<Timer> timer);
private:
    // 当系统时间改变时 -> 调用该函数
    bool detectClockRollover();

private:
    std::shared_mutex m_mutex;
    // 时间堆
    std::set<std::shared_ptr<Timer>, Timer::Comparator> m_timers;
    // 在下次getNextTime()执行前 onTimerInsertedAtFront()是否已经被触发了 -> 在此过程中 onTimerInsertedAtFront()只执行一次
    bool m_tickled = false;
    // 上次检查系统时间是否回退的绝对时间
    std::chrono::time_point<std::chrono::system_clock> m_previouseTime;
};


};



#endif
