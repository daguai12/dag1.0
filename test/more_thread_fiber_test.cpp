#include <iostream>
#include <memory>
#include <thread>
#include "fiber.h"

using namespace dag;

void threadA_scheduler() {
    // 创建主协程
    std::cout << "Thread A started with ID: " << std::this_thread::get_id() << std::endl;
    Fiber::GetThis();

    //创建协程A1 和 A2
    std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>([](){
        std::cout << "Coroutine A1 running" << std::endl;
        std::cout << "Coroutine A1 Id" << Fiber::GetFiberId() << std::endl;
    });
    fiber->resume();
}

void threadB_scheduler() {
    // 创建主协程
    std::cout << "Thread A started with ID: " << std::this_thread::get_id() << std::endl;
    Fiber::GetThis();

    //创建协程A1 和 A2
    std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>([](){
        std::cout << "Coroutine A1 running" << std::endl;
        std::cout << "Coroutine A1 Id" << Fiber::GetFiberId() << std::endl;
    });

    fiber->resume();
}

int main()
{
    std::thread t1(threadA_scheduler);
    std::thread t2(threadB_scheduler);
    t1.join();
    t2.join();
    return 0;
}
