#ifndef _THREAD_H_
#define _THREAD_H_

#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>


namespace dag
{

// 实现线程同步
class Semaphore
{
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
public:
    explicit Semaphore(int count_ = 0) : count(count_) {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock,[&](){
            return count == 0;
        });
        count--;
    }

    void signal()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};

// 一共两种线程： 1 由系统自动创建的线程 2 由Thread类创建的线程
class Thread
{
public:
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void join();

public:
    //获取系统分配的线程ID
    // static pid_t GetThreadId();
    //获取当前所在线程
    static Thread* GetThis();

    //获取当前线程的名字
    static const std::string& GetName();
    //设置当前线程的名字
    static void SetName(const std::string& name);

private:
    //线程函数
    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;

    //线程需要运行的函数
    std::function<void()> m_cb;
    std::string m_name;

    Semaphore m_semaphore;
};

}

#endif
