#ifndef _DG_SINGLETON_H_
#define _DG_SINGLETON_H_

#include "noncopyable.h"
#include <arpa/inet.h>
#include <mutex>

namespace dag {

#if 1
template <typename T>
class Singleton : public dag::NonCopyable
{
private:
    static T* instance;
    static std::mutex mutex;
protected: 
    Singleton() {}
public:
    Singleton(const Singleton& ) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static T* GetInstance()
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (instance == nullptr)
        {
            instance = new T();
        }
        return instance;
    }

    static void DestroyInstance()
    {
        std::lock_guard<std::mutex> lock(mutex);
        delete instance;
        instance = nullptr;
    }
};

template <typename T>
std::mutex Singleton<T>::mutex;

template <typename T>
T* Singleton<T>::instance = nullptr;

#endif



#if 0
template <typename T> 
class Singleton : public dag::NonCopyable
{
public:
    static T& GetInstance()
    {
        static Singleton instance = new T();
        return instance;
    }

private:
    Singleton() {

    }
};

#endif
};


#endif
