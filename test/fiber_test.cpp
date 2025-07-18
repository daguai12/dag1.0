#include "fiber.h"
#include <vector>
#include <iostream>

using namespace dag;

class Scheduler
{
public:
    //添加协程调度任务
    void scheduler(std::shared_ptr<Fiber> task)
    {
        m_tasks.push_back(task);
    }

    //执行调度任务
    void run()
    {
        std::cout << "number " << m_tasks.size() << std::endl;

        std::shared_ptr<Fiber> task;
        auto it = m_tasks.begin();
        while(it != m_tasks.end())
        {
            task = *it;
            task->resume();
            it++;
        }
        m_tasks.clear();
    }
private:
    //任务队列
    std::vector<std::shared_ptr<Fiber>> m_tasks;
};

void test_fiber(int i)
{
    std::cout << "hello world" << i << std::endl;
}

void test1()
{
    // 初始化当前线程的主协程
    Fiber::GetThis();
    //  创建调度器
    Scheduler sc;

    //添加调度人物（任务和子线程绑定）
    for(auto i = 0;i < 20;i++)
    {
        //创建子协程
        //使用共享指针自动管理资源 -> 过期后自动释放子协程创建的资源
        //begin函数 -> 绑定函数和参数用来返回一个函数对象
        std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(std::bind(test_fiber,i),0,false);
        sc.scheduler(fiber);
    }

    //执行调度任务
    sc.run();
}


void test2()
{
    Fiber::GetThis();
    std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>([](){
        std::cout << "开始协程1" << std::endl;
        Fiber::GetThis()->yield();
        std::cout << "开始协程3" << std::endl;
    },0,false);
    std::shared_ptr<Fiber> fiber2 = std::make_shared<Fiber>([](){
        std::cout << "开始协程2" << std::endl;
        Fiber::GetThis()->yield();
        std::cout << "开始协程4" << std::endl;
    },0,false);
    fiber->resume();
    fiber2->resume();
    fiber->resume();
    fiber2->resume();
}

void test3()
{
    Fiber::GetThis();
    std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>([](){
        std::cout << Fiber::GetFiberId() << std::endl;
        std::cout << "开始协程1" << std::endl;
        Fiber::GetThis()->yield();
        std::cout << "开始协程2" << std::endl;
    });

    std::shared_ptr<Fiber> fiber2 = std::make_shared<Fiber>([](){
        std::cout << Fiber::GetFiberId() << std::endl;
        std::cout << "开始协程3" << std::endl;
        Fiber::GetThis()->yield();
        std::cout << "开始协程4" << std::endl;
    });
    fiber->resume();
    fiber2->resume();
    fiber->resume();
    fiber2->resume();
}



int main()
{
    test2();
    test3();
    return 0;
}

