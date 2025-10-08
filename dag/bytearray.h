#ifndef __DAG_BYTEARRAY_H__
#define __DAG_BYTEARRAY_H__

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include "hook.h"

namespace dag {

class ByteArray {
public:
    using ptr = std::shared_ptr<ByteArray>;

    /*
    * @brief ByteArray的存储节点
    */
    struct Node {
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;
        size_t size;
        Node* next;
    };


    ByteArray(size_t base_size = 4096);

    ~ByteArray();

    
    //写入定长整数
    void writeFint8(int8_t  value);
    void writeFuint8(uint8_t value);
    void writeFint16(int16_t  value);
    void writeFuint16(uint16_t value);
    void writeFint32(int32_t  value);
    void writeFuint32(uint32_t value);
    void writeFint64(int64_t  value);
    void writeFuint64(uint64_t value);


    //写入varint编码整数
    void writeInt32(int32_t  value);
    void writeUint32(uint32_t value);
    void writeInt64(int64_t  value);
    void writeUint64(uint64_t value);


    void writeFloat  (float value);

    void writeDouble (double value);

    //写入字符串
    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);
    void writeStringVint(const std::string& value);

    void writeStringWithoutLength(const std::string& value);




    //读取固定格式的整数
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();


    //读取变长编码格式的整数
    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    //读取浮点数格式
    float readFloat();
    double readDouble();


    //length:int16, data
    std::string readStringF16();
    //length:int32, data
    std::string readStringF32();
    //length:int64, data
    std::string readStringF64();
    //length:varint, data
    std::string readStringVint();





    //内部操作
    // 清除bytearray
    void clear();

    // 将数据写入到node节点中
    void write(const void* buf, size_t size);

    // 将node中的数据写入到buf中
    void read(void* buf, size_t size);

    // 将指定位置的数据读入buf中
    void read(void* buf, size_t size, size_t position) const;

    // 获取当前的position
    size_t getPosition() const { return m_position;}

    // 将当前位置指针移动到位置 v
    void setPosition(size_t v);

    // 将node中的数据写入到File中
    bool writeToFile(const std::string& name) const;

    // 将FIle中的文件读入到Node中
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const { return m_baseSize;}

    size_t getReadSize() const { return m_size - m_position;}

    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);


    std::string toString() const;
    std::string toHexString() const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers
                                ,uint64_t len, uint64_t position) const;



    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    size_t getSize() const {return m_size;}

private:
    // 扩容ByteArray,使其可以容纳size个数据
    void addCapacity(size_t size);
    // 获取当前可写容量
    size_t getCapacity() const { return m_capacity - m_position;}

private:
    //内存块的大小
    size_t m_baseSize;
    //当前操作位置
    size_t m_position;
    //当前的总容量
    size_t m_capacity;
    //当前数据的大小
    size_t m_size;
    //字节序，默认大端法
    int8_t m_endian;
    // 第一个内存块指针
    Node* m_root;
    // 当前操作的内存指针
    Node* m_cur;
};

}
#endif

