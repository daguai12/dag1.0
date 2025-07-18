#include "fiber.h"
#include <iostream>
#include <list>

/**
 * @brief 简单的协程调度类，支持添加调度任务已经运行调度任务
*/
class Scheduler {
public:
    /**
     * @brief 添加协程调度任务
    */
    void scheduelr(dag::Fiber::ptr task) {
        m_tasks.push_back(task);
    }

    /**
     * @brief 执行调度任务
    */
    void run() {
        dag::Fiber::ptr task;
        auto it = m_tasks.begin();
        while(it != m_tasks.end()) {
            task = *it;
            m_tasks.erase(it++);
            task->resume();
        }
    }
private:
    // 任务队列
    std::list<dag::Fiber::ptr> m_tasks;
};

void test_fiber(int i) {
    std::cout << "hello world"  << i << std::endl;
}

int main()
{
    // 初始化当前线程的主协程
    dag::Fiber::GetThis();

    // 创建协程调度器
    Scheduler sc;

    // 添加调度任务
    for (auto i = 0;i < 10;++i){
        sc.scheduelr(std::make_shared<dag::Fiber>([=](){
            std::cout << "hello world" << i << std::endl;
        }));
    }

    // 执行调度任务
    sc.run();
    return 0;
}

