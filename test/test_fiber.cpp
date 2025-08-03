#include "fiber.h"
#include <iostream>
#include <memory>
#include <sched.h>
#include <functional>
#include <vector>

class Scheduler {
public:
    Scheduler(){}

    void scheduler(std::shared_ptr<dag::Fiber> cb) {
        m_tasks.push_back(cb);
    }

    void run() {
        std::cout << "number " << m_tasks.size() << std::endl;

        std::shared_ptr<dag::Fiber> task;
        auto it = m_tasks.begin();
        while(it != m_tasks.end()) {
            task = *it;
            task->resume();
            it++;
        }
        m_tasks.clear();
    }
private:
    std::vector<std::shared_ptr<dag::Fiber>> m_tasks;
};



void test1() {
    // 初始化主协程
    dag::Fiber::GetThis();
    // 创建用户协程
    std::shared_ptr<dag::Fiber> fiber1(new dag::Fiber([](){
        std::cout << "I'm fiber1" << std::endl;
    }));
    

    fiber1->resume();
    fiber1->reset([](){
        std::cout << "I'm fiber2" << std::endl;
    });
    fiber1->resume();
}

void test_fiber(int i) {
    std::cout << "hello world" << i << std::endl;
}

void test2() {
    dag::Fiber::GetThis();

    Scheduler sc;
    for(int i = 0;i < 10;++i) {
        std::shared_ptr<dag::Fiber> fiber = std::make_shared<dag::Fiber>(std::bind(test_fiber,i),0,false);
        sc.scheduler(fiber);
    }
    sc.run();
}


int main()
{
    test1();
    test2();
    return 0;
}
