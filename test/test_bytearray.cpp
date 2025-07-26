#if 0
#include "bytearray.h"
#include "logger.h"
#include "../fiber_lib/utils/asserts.h"
#include <cstdint>
#include <iostream>



static dag::Logger::ptr g_logger = DAG_LOG_ROOT();


void test() {
#define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    dag::ByteArray::ptr ba(new dag::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    ba->setPosition(0); \
    for(size_t i = 0; i < vec.size(); ++i) { \
        type v = ba->read_fun(); \
        std::cout << "Read[" << i << "] = "  << (int64_t)v << std::endl;\
        DAG_ASSERT(v == vec[i]); \
    } \
    DAG_ASSERT(ba->getReadSize() == 0); \
    DAG_LOG_INFO(g_logger) << #write_fun "/" #read_fun \
                    " (" #type " ) len=" << len \
                    << " base_len=" << base_len \
                    << " size=" << ba->getSize(); \
}

    XX(int8_t,  100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t,  100, writeFint16,  readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t,  100, writeFint32,  readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t,  100, writeFint64,  readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t,  100, writeInt32,  readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t,  100, writeInt64,  readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    dag::ByteArray::ptr ba(new dag::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    ba->setPosition(0); \
    for(size_t i = 0; i < vec.size(); ++i) { \
        type v = ba->read_fun(); \
        DAG_ASSERT(v == vec[i]); \
    } \
    DAG_ASSERT(ba->getReadSize() == 0); \
    DAG_LOG_INFO(g_logger) << #write_fun "/" #read_fun \
                    " (" #type " ) len=" << len \
                    << " base_len=" << base_len \
                    << " size=" << ba->getSize(); \
    ba->setPosition(0); \
    DAG_ASSERT(ba->writeToFile("/tmp/" #type "_" #len "-" #read_fun ".dat")); \
    dag::ByteArray::ptr ba2(new dag::ByteArray(base_len * 2)); \
    DAG_ASSERT(ba2->readFromFile("/tmp/" #type "_" #len "-" #read_fun ".dat")); \
    ba2->setPosition(0); \
    DAG_ASSERT(ba->toString() == ba2->toString()); \
    DAG_ASSERT(ba->getPosition() == 0); \
    DAG_ASSERT(ba2->getPosition() == 0); \
}
    XX(int8_t,  100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t,  100, writeFint16,  readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t,  100, writeFint32,  readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t,  100, writeFint64,  readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t,  100, writeInt32,  readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t,  100, writeInt64,  readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);

#undef XX
}

int main(int argc, char** argv) {
    test();
    return 0;
}
#endif


       #if 0
#include "bytearray.h"
#include <iostream>
#include <cassert>

void testStringIO() {
    dag::ByteArray::ptr ba(new dag::ByteArray());

    std::string s1 = "hello world";
    std::string s2 = "this is a test";
    std::string s3 = "unlength string";

    // 写入字符串（带长度）
    ba->writeStringF16(s1);
    ba->writeStringF32(s2);

    // 写入字符串（不带长度）
    ba->writeStringWithoutLength(s3);

    // 重置 position
    ba->setPosition(0);

    // 读取带长度的字符串
    std::string r1 = ba->readStringF16();
    std::string r2 = ba->readStringF32();

    // 读取不带长度的字符串，需要手动知道长度
    char buf[100] = {0};
    ba->read(buf, s3.size()); // 读取指定长度
    std::string r3(buf, s3.size());

    // 验证
    assert(r1 == s1);
    assert(r2 == s2);
    assert(r3 == s3);

    std::cout << "r1: " << r1 << std::endl;
    std::cout << "r2: " << r2 << std::endl;
    std::cout << "r3: " << r3 << std::endl;
}

int main() {
    testStringIO();
    return 0;
}
       #endif



#include "bytearray.h"
#include <iostream>
#include <sys/uio.h>
#include <cstring>
#include <cassert>

// 模拟 "socket send" 过程：将 write_buffers 中的数据复制到 recv_buffer 中
void simulateSend(const std::vector<iovec>& write_buffers, char* recv_buffer, size_t total_len) {
    size_t copied = 0;
    for (const auto& iov : write_buffers) {
        memcpy(recv_buffer + copied, iov.iov_base, iov.iov_len);
        copied += iov.iov_len;
    }
    assert(copied == total_len); // 验证长度匹配
}

// 模拟 "socket recv" 过程：将 recv_buffer 的内容填充到另一个 ByteArray
void simulateRecv(dag::ByteArray::ptr dst_ba, const char* recv_buffer, size_t len) {
    // 分配写空间
    std::vector<iovec> read_buffers;
    dst_ba->getWriteBuffers(read_buffers, len);

    size_t copied = 0;
    for (auto& iov : read_buffers) {
        memcpy(iov.iov_base, recv_buffer + copied, iov.iov_len);
        copied += iov.iov_len;
    }
    dst_ba->setPosition(dst_ba->getPosition() + len);
}

int main() {
    std::string msg = "hello ByteArray getReadBuffers/getWriteBuffers";

    // 发送端 ByteArray
    dag::ByteArray::ptr send_ba(new dag::ByteArray(10)); // 小 baseSize 强制分多块
    send_ba->writeStringF32(msg);
    send_ba->setPosition(0); // 准备发送

    // 获取 write buffers
    std::vector<iovec> write_buffers;
    size_t send_len = send_ba->getReadBuffers(write_buffers, send_ba->getSize(), 0);

    // 模拟发送数据到中间 buffer（类似 socket 中转）
    char* middle_buffer = new char[send_len];
    simulateSend(write_buffers, middle_buffer, send_len);

    // 接收端 ByteArray
    dag::ByteArray::ptr recv_ba(new dag::ByteArray(10));
    simulateRecv(recv_ba, middle_buffer, send_len);
    recv_ba->setPosition(0); // 准备读取

    std::string recv_msg = recv_ba->readStringF32();

    std::cout << "Sent:    " << msg << std::endl;
    std::cout << "Received:" << recv_msg << std::endl;

    assert(msg == recv_msg);
    std::cout << "✅ Test passed!" << std::endl;

    delete[] middle_buffer;
    return 0;
}
