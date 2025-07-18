#ifndef _DAG_MUTEX_H_
#define _DAG_MUTEX_H_

#include <atomic>
#include <thread>
#include "noncopyable.h"

namespace dag {
class SpinLock : public NonCopyable{
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    bool try_lock() {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
    
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

};

#endif


