// #ifndef __DAG_BYTEARRAY_H__
// #define __DAG_BYTEARRAY_H__
//
// #include <cstdint>
// #include <memory>
// #include <vector>
// #include <string>
// #include <stdint.h>
// #include <sys/uio.h>
// #include <sys/types.h>
//
// namespace dag {
//
// class ByteArray {
// public:
//     using ptr = std::shared_ptr<ByteArray>;
//
//     /*
//     * @brief ByteArray的存储节点
//     */
//     struct Node {
//         Node(size_t s);
//         Node();
//         ~Node();
//
//         char* ptr;
//         size_t size;
//         Node* next;
//     };
//
//
//     ByteArray(size_t base_size = 4096);
//
//     ~ByteArray();
//
//
//     //写入定长整数
//     void writeFint8(int8_t  value);
//     void writeFuint8(uint8_t value);
//     void writeFint16(int16_t  value);
//     void writeFuint16(uint16_t value);
//     void writeFint32(int32_t  value);
//     void writeFuint32(uint32_t value);
//     void writeFint64(int64_t  value);
//     void writeFuint64(uint64_t value);
//
//
//     //写入varint编码整数
//     void writeInt32(int32_t  value);
//     void writeUint32(uint32_t value);
//     void writeInt64(int64_t  value);
//     void writeUint64(uint64_t value);
//
//
//     void writeFloat  (float value);
//
//     void writeDouble (double value);
//
//     //写入字符串
//     void writeStringF16(const std::string& value);
//     void writeStringF32(const std::string& value);
//     void writeStringF64(const std::string& value);
//     void writeStringVint(const std::string& value);
//
//     void writeStringWithoutLength(const std::string& value);
//
//
//
//
//     //读取固定格式的整数
//     int8_t readFint8();
//     uint8_t readFuint8();
//     int16_t readFint16();
//     uint16_t readFuint16();
//     int32_t readFint32();
//     uint32_t readFuint32();
//     int64_t readFint64();
//     uint64_t readFuint64();
//
//
//     //读取变长编码格式的整数
//     int32_t readInt32();
//     uint32_t readUint32();
//     int64_t readInt64();
//     uint64_t readUint64();
//
//     //读取浮点数格式
//     float readFloat();
//     double readDouble();
//
//
//     //length:int16, data
//     std::string readStringF16();
//     //length:int32, data
//     std::string readStringF32();
//     //length:int64, data
//     std::string readStringF64();
//     //length:varint, data
//     std::string readStringVint();
//
//
//
//
//
//     //内部操作
//     // 清除bytearray
//     void clear();
//
//     // 将数据写入到node节点中
//     void write(const void* buf, size_t size);
//
//     // 将node中的数据写入到buf中
//     void read(void* buf, size_t size);
//
//     // 将指定位置的数据读入buf中
//     void read(void* buf, size_t size, size_t position) const;
//
//     // 获取当前的position
//     size_t getPosition() const { return m_position;}
//
//     // 将当前位置指针移动到位置 v
//     void setPosition(size_t v);
//
//     // 将node中的数据写入到File中
//     bool writeToFile(const std::string& name) const;
//
//     // 将FIle中的文件读入到Node中
//     bool readFromFile(const std::string& name);
//
//     size_t getBaseSize() const { return m_baseSize;}
//
//     size_t getReadSize() const { return m_size - m_position;}
//
//     bool isLittleEndian() const;
//     void setIsLittleEndian(bool val);
//
//
//     std::string toString() const;
//     std::string toHexString() const;
//
//     uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;
//
//     uint64_t getReadBuffers(std::vector<iovec>& buffers
//                                 ,uint64_t len, uint64_t position) const;
//
//
//
//     uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
//
//     size_t getSize() const {return m_size;}
//
// private:
//     // 扩容ByteArray,使其可以容纳size个数据
//     void addCapacity(size_t size);
//     // 获取当前可写容量
//     size_t getCapacity() const { return m_capacity - m_position;}
//
// private:
//     //内存块的大小
//     size_t m_baseSize;
//     //当前操作位置
//     size_t m_position;
//     //当前的总容量
//     size_t m_capacity;
//     //当前数据的大小
//     size_t m_size;
//     //字节序，默认大端法
//     int8_t m_endian;
//     // 第一个内存块指针
//     Node* m_root;
//     // 当前操作的内存指针
//     Node* m_cur;
// };
//
// }
// #endif


#ifndef __DAG_BYTEARRAY_H__
#define __DAG_BYTEARRAY_H__

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/types.h>

namespace dag {

/**
 * @brief 序列化/反序列化模块，提供对二进制数据的高效读写
 * @details 内部使用链式内存块(Node)来管理内存，避免在扩容时进行大规模内存拷贝
 */
class ByteArray {
public:
    using ptr = std::shared_ptr<ByteArray>;

    /**
     * @brief ByteArray的内部存储节点
     */
    struct Node {
        /**
         * @brief 构造函数，分配指定大小的内存块
         * @param[in] s 内存块大小
         */
        Node(size_t s);
        /**
         * @brief 默认构造函数
         */
        Node();
        /**
         * @brief 析构函数，释放内存块
         */
        ~Node();

        char* ptr;      // 内存块指针
        size_t size;    // 内存块大小
        Node* next;     // 指向下一个节点的指针
    };

    /**
     * @brief 构造函数
     * @param[in] base_size 内部内存块(Node)的基准大小，默认为4096字节
     */
    ByteArray(size_t base_size = 10240);

    /**
     * @brief 析构函数，释放所有内存块
     */
    ~ByteArray();

    //======================= 写入接口 =======================

    /**
     * @brief 写入一个8位有符号整数
     * @param[in] value 待写入的值
     */
    void writeFint8(int8_t  value);
    /**
     * @brief 写入一个8位无符号整数
     * @param[in] value 待写入的值
     */
    void writeFuint8(uint8_t value);
    /**
     * @brief 写入一个16位有符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFint16(int16_t  value);
    /**
     * @brief 写入一个16位无符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFuint16(uint16_t value);
    /**
     * @brief 写入一个32位有符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFint32(int32_t  value);
    /**
     * @brief 写入一个32位无符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFuint32(uint32_t value);
    /**
     * @brief 写入一个64位有符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFint64(int64_t  value);
    /**
     * @brief 写入一个64位无符号整数(会进行字节序转换)
     * @param[in] value 待写入的值
     */
    void writeFuint64(uint64_t value);

    /**
     * @brief 以Varint+ZigZag格式写入一个32位有符号整数
     * @param[in] value 待写入的值
     */
    void writeInt32(int32_t  value);
    /**
     * @brief 以Varint格式写入一个32位无符号整数
     * @param[in] value 待写入的值
     */
    void writeUint32(uint32_t value);
    /**
     * @brief 以Varint+ZigZag格式写入一个64位有符号整数
     * @param[in] value 待写入的值
     */
    void writeInt64(int64_t  value);
    /**
     * @brief 以Varint格式写入一个64位无符号整数
     * @param[in] value 待写入的值
     */
    void writeUint64(uint64_t value);

    /**
     * @brief 写入一个float类型数据
     * @param[in] value 待写入的值
     */
    void writeFloat(float value);
    /**
     * @brief 写入一个double类型数据
     * @param[in] value 待写入的值
     */
    void writeDouble(double value);

    /**
     * @brief 写入一个前缀为16位长度的字符串
     * @param[in] value 待写入的字符串
     */
    void writeStringF16(const std::string& value);
    /**
     * @brief 写入一个前缀为32位长度的字符串
     * @param[in] value 待写入的字符串
     */
    void writeStringF32(const std::string& value);
    /**
     * @brief 写入一个前缀为64位长度的字符串
     * @param[in] value 待写入的字符串
     */
    void writeStringF64(const std::string& value);
    /**
     * @brief 写入一个前缀为Varint编码长度的字符串
     * @param[in] value 待写入的字符串
     */
    void writeStringVint(const std::string& value);
    /**
     * @brief 直接写入字符串数据，不带长度前缀
     * @param[in] value 待写入的字符串
     */
    void writeStringWithoutLength(const std::string& value);

    //======================= 读取接口 =======================

    /**
     * @brief 读取一个8位有符号整数
     * @return 读取到的值
     */
    int8_t readFint8();
    /**
     * @brief 读取一个8位无符号整数
     * @return 读取到的值
     */
    uint8_t readFuint8();
    /**
     * @brief 读取一个16位有符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    int16_t readFint16();
    /**
     * @brief 读取一个16位无符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    uint16_t readFuint16();
    /**
     * @brief 读取一个32位有符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    int32_t readFint32();
    /**
     * @brief 读取一个32位无符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    uint32_t readFuint32();
    /**
     * @brief 读取一个64位有符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    int64_t readFint64();
    /**
     * @brief 读取一个64位无符号整数(会进行字节序转换)
     * @return 读取到的值
     */
    uint64_t readFuint64();

    /**
     * @brief 读取一个以Varint+ZigZag格式编码的32位有符号整数
     * @return 读取到的值
     */
    int32_t readInt32();
    /**
     * @brief 读取一个以Varint格式编码的32位无符号整数
     * @return 读取到的值
     */
    uint32_t readUint32();
    /**
     * @brief 读取一个以Varint+ZigZag格式编码的64位有符号整数
     * @return 读取到的值
     */
    int64_t readInt64();
    /**
     * @brief 读取一个以Varint格式编码的64位无符号整数
     * @return 读取到的值
     */
    uint64_t readUint64();

    /**
     * @brief 读取一个float类型数据
     * @return 读取到的值
     */
    float readFloat();
    /**
     * @brief 读取一个double类型数据
     * @return 读取到的值
     */
    double readDouble();

    /**
     * @brief 读取一个前缀为16位长度的字符串
     * @return 读取到的字符串
     */
    std::string readStringF16();
    /**
     * @brief 读取一个前缀为32位长度的字符串
     * @return 读取到的字符串
     */
    std::string readStringF32();
    /**
     * @brief 读取一个前缀为64位长度的字符串
     * @return 读取到的字符串
     */
    std::string readStringF64();
    /**
     * @brief 读取一个前缀为Varint编码长度的字符串
     * @return 读取到的字符串
     */
    std::string readStringVint();

    //======================= 内部操作 =======================
    /**
     * @brief 清空ByteArray中的所有数据，并重置所有状态
     */
    void clear();

    /**
     * @brief 写入指定长度的数据
     * @param[in] buf 数据来源缓冲区
     * @param[in] size 要写入的字节数
     */
    void write(const void* buf, size_t size);

    /**
     * @brief 从当前位置读取指定长度的数据
     * @param[out] buf 用于接收数据的缓冲区
     * @param[in] size 要读取的字节数
     * @throw std::out_of_range 如果可读数据不足
     */
    void read(void* buf, size_t size);

    /**
     * @brief 从指定位置读取指定长度的数据
     * @param[out] buf 用于接收数据的缓冲区
     * @param[in] size 要读取的字节数
     * @param[in] position 开始读取的逻辑位置
     * @throw std::out_of_range 如果可读数据不足
     */
    void read(void* buf, size_t size, size_t position) const;

    /**
     * @brief 获取当前的逻辑读写位置
     * @return size_t 当前位置
     */
    size_t getPosition() const { return m_position;}

    /**
     * @brief 设置当前的逻辑读写位置
     * @param[in] v 新的位置
     * @throw std::out_of_range 如果v超过总容量
     */
    void setPosition(size_t v);

    /**
     * @brief 将ByteArray中可读的数据写入到文件中
     * @param[in] name 文件名
     * @return 是否写入成功
     */
    bool writeToFile(const std::string& name) const;

    /**
     * @brief 从文件中读取数据并写入到ByteArray
     * @param[in] name 文件名
     * @return 是否读取成功
     */
    bool readFromFile(const std::string& name);

    /**
     * @brief 获取内部内存块的基准大小
     * @return size_t
     */
    size_t getBaseSize() const { return m_baseSize;}

    /**
     * @brief 获取剩余可读的数据大小
     * @return size_t
     */
    size_t getReadSize() const { return m_size - m_position;}

    /**
     * @brief 获取当前的字节序是否为小端
     * @return bool
     */
    bool isLittleEndian() const;
    /**
     * @brief 设置字节序
     * @param[in] val true为小端，false为大端
     */
    void setIsLittleEndian(bool val);

    /**
     * @brief 将可读数据转换为字符串
     * @return std::string
     */
    std::string toString() const;
    /**
     * @brief 将可读数据转换为16进制字符串(带格式)
     * @return std::string
     */
    std::string toHexString() const;

    /**
     * @brief 从当前位置获取可读数据的iovec数组(用于Scatter/Gather IO)
     * @param[out] buffers 接收iovec的数组
     * @param[in] len 要获取的最大长度
     * @return 实际获取到的数据总长度
     * @note 此接口可以避免内存拷贝，直接用于writev等系统调用
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;
    
    /**
     * @brief 从指定位置获取可读数据的iovec数组(用于Scatter/Gather IO)
     * @param[out] buffers 接收iovec的数组
     * @param[in] len 要获取的最大长度
     * @param[in] position 开始获取的逻辑位置
     * @return 实际获取到的数据总长度
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;

    /**
     * @brief 获取可写空间的iovec数组
     * @param[out] buffers 接收iovec的数组
     * @param[in] len 要获取的可写空间长度
     * @return 实际获取到的空间总长度
     * @note 此接口可以避免内存拷贝，直接用于readv等系统调用
     */
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    /**
     * @brief 获取当前已写入的数据大小
     * @return size_t
     */
    size_t getSize() const {return m_size;}

private:
    /**
     * @brief 扩容ByteArray,使其可以容纳至少size个新数据
     * @param[in] size 需要增加的最小容量
     */
    void addCapacity(size_t size);
    /**
     * @brief 获取当前可写的剩余容量
     * @return size_t
     */
    size_t getCapacity() const { return m_capacity - m_position;}

private:
    /// 内存块的基准大小
    size_t m_baseSize;
    /// 当前操作的逻辑位置
    size_t m_position;
    /// 当前的总容量
    size_t m_capacity;
    /// 当前已写入数据的大小
    size_t m_size;
    /// 字节序，默认为大端
    int8_t m_endian;
    /// 第一个内存块指针
    Node* m_root;
    /// 当前操作的内存块指针
    Node* m_cur;
};

}
#endif
