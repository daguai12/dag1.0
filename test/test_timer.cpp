#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <thread>
#include <memory>
#include <vector>

#include "timer.h"

using namespace dag;

void printCurrentTime(const std::string& tag) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << tag << "] " << std::ctime(&now_time);
}

int main()
{
    TimerManager manager;

    std::cout << "=== TimerManager Test Begin ===" << std::endl;

    auto one_shot_timer = manager.addTimer(150, [](){
        printCurrentTime("One_shot_timer fired") ;
    },false);

    auto recurring_timer = manager.addTimer(200, [](){
        printCurrentTime("Recurring timer fired");
    },true);

    auto condition_holder = std::make_shared<int>(42);
    std::weak_ptr<void> weak_cond = condition_holder;

    manager.addConditionTimer(300, [](){
        printCurrentTime("ConditionTimer (alive) fired");
    }, weak_cond, false);

    {
        auto temp_ptr = std::make_shared<int>(100);
        std::weak_ptr<void> expired_weak = temp_ptr;
        manager.addConditionTimer(400, [](){
            printCurrentTime("ConditionTimer (expired) should NOT fire!");
        }, expired_weak,false);
    }

    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\n[Main Loop] Tick" << i + 1 << std::endl;

        std::vector<std::function<void()>> cbs;
        manager.listExpiredCb(cbs);

        for (auto& cb : cbs) {
            cb();
        }
    }

    std::cout << "=== TimerManager Test End ===" << std::endl;
    return 0;
}


