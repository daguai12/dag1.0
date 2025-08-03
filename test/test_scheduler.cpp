#include <memory>
#include <iostream>
#include <unistd.h>
#include "scheduler.h"
#include "./utils/util.h"

using namespace dag;

static unsigned int test_number;

std::mutex mutex_cout;

void task()
{
    {
        std::lock_guard<std::mutex> lock(mutex_cout);
        std::cout << "task " << test_number++ << " is under processing in thread: " << getThreadId() << std::endl;
    }
    sleep(1);
}

int main(int argc,char* argv[])
{
    {
        std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>(1,true,"scheduler_1");

        scheduler->start();

        sleep(2);

        std::cout << "\nbegin post\n\n";

        for(int i = 0;i < 5;i++)
        {
            std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(task);
            scheduler->schedulerLock(fiber);
        }

        sleep(6);

        std::cout << "\npost again\n\n";
        for(int i = 0;i < 15;i++)
        {
            std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(task);
            scheduler->schedulerLock(fiber);
        }

        sleep(3);
        //scheduler如果没有设置加入工作处理
        scheduler->stop();
    }
    return 0;
}




