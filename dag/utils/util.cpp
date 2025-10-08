#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <execinfo.h>
#include <sstream>
#include <iostream>
#include <openssl/sha.h>
#include "fiber.h"
#include "util.h"

namespace dag{


/**
 * @brief 获得系统从开始运行到现在过去的时间
 */
uint64_t getElapseMs()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

uint32_t getThreadId() {
    return syscall(SYS_gettid);
}

uint32_t getFiberId() {
    return Fiber::GetFiberId();
}


std::string getThreadName() {
    // 系统调用要求不能超过 16 字节
    char thread_name[16];
    pthread_getname_np(pthread_self(), thread_name, 16);
    return thread_name;
}

void setThreadName(const std::string &name) {
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
}

uint64_t getCurrentTime() {
    struct timeval val{};
    gettimeofday(&val, nullptr);
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

void backtrace(std::vector<std::string> &bt, int size, int skip) {
    void **array = (void **) ::malloc(sizeof(void *) * size);
    int s = ::backtrace(array, size);

    char **strings = ::backtrace_symbols(array, size);
    if (strings == nullptr) {
        std::cerr << "backtrace_symbols error" << std::endl;
    }

    for (int i = skip; i < s; ++i) {
        bt.emplace_back(strings[i]);
    }
    ::free(array);
    ::free(strings);
}

std::string backtraceToString(int size, int skip, const std::string &prefix) {

    std::vector<std::string> bt;
    backtrace(bt, size, skip);
    std::stringstream ss;
    for (const auto &i: bt) {
        ss << prefix << " " << i << std::endl;
    }
    return ss.str();
}
};
