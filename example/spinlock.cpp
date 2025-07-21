#if 0
#include "bytearray.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <iomanip>
#include <vector>

#include "endian.h"
#include "logger.h"

namespace dag {

static dag::Logger::ptr g_logger = DAG_LOG_NAME("system");

ByteArray::Node::Node(size_t s) 
    :ptr(new char[s])
    ,size(s)
    ,next(nullptr) 
{

}

ByteArray::Node::Node()
    :ptr(nullptr)
    ,size(0)
    ,next(nullptr)
{

}

ByteArray::Node::~Node(){
    if(ptr) {
        delete[] ptr; 
    }
}


ByteArray::ByteArray(size_t base_size) 
    : m_baseSize(base_size)
    , m_position(0)
    , m_capacity(base_size)
    , m_size(0)
    , m_endian() 
    , m_root(new Node(base_size))
    , m_cur(m_root)
{

}

ByteArray::~ByteArray(){
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

bool ByteArray::isLittleEndian() const {
    return m_endian == DAG_LITTLE_ENDIAN;
}


void ByteArray::setIsLittleEndian(bool val) {
    if(val) {
        m_endian = DAG_LITTLE_ENDIAN;
    } else {
        m_endian = DAG_BIG_ENDIAN;
    }
}

// ============================
//      writeFint(定长格式)
// ============================

void ByteArray::writeFint8(int8_t  value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t  value) {
    if(m_endian != DAG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if(m_endian != DAG_BYTE_ORDER){
        value = byteswap(value);
    }
    write(&value, sizeof(value));

}

void ByteArray::writeFint32(int32_t  value){
    if(m_endian != DAG_BYTE_ORDER){
        value = byteswap(value);
    }
    write(&value, sizeof(value));

}

void ByteArray::writeFuint32(uint32_t value){
    if(m_endian != DAG_BYTE_ORDER){
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t  value){
    if(m_endian != DAG_BYTE_ORDER){
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value){
    if(m_endian != DAG_BYTE_ORDER){
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}


// ============================
//  Zigzag算法
// ============================

/**
* @brief 使用Zigzag将有符号整数编码为无符号整数
*/ 
static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

/**
* @brief 使用Zigzag将有符号整数编码为无符号整数
*/
static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

/**
*@brief 将Zigzag编码过的整数还原为无符号整数
*/
static int32_t DecodeZigzag32(const int32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

/**
*@brief 将Zigzag编码过的整数还原为无符号整数
*/
static int64_t DecodeZigzag64(const int64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

// =============================
//      writeUint/writeInt (varint变长编码)
// =============================


void ByteArray::writeInt32(int32_t value){
    writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32(uint32_t value){
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        // tmp[0] = 
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp,i);

}

void ByteArray::writeInt64(int64_t  value){
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value){
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7f) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}


// =============================
//      writeFloat
// =============================
void ByteArray::writeFloat(float& value){
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble(double& value){
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}



// =============================
//      writeString
// =============================
// 不需要转换字节序，字符为单字节
void ByteArray::writeStringF16(std::string& value){
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(std::string& value){
    writeFint32(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringF64(std::string& value){
    writeFint64(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringVint(std::string value){
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(std::string& value){
    write(value.c_str(),  value.size());
}



// ===========================
//          read
// ===========================
int8_t ByteArray::readFint8() {
    int8_t v;
    read(&v,sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v,sizeof(v));\
    if(m_endian == DAG_BYTE_ORDER) { \
        return v; \
    } else { \
        return byteswap(v);\
    } 

int16_t ByteArray::readFint16(){
    XX(int16_t);
}

uint16_t ByteArray::readFuint16(){
    XX(uint16_t);
}

int32_t ByteArray::readFint32(){
    XX(int32_t);
}

uint32_t ByteArray::readFuint32(){
    XX(uint32_t);
}

int64_t ByteArray::readFint64(){
    XX(int32_t);
}

uint64_t ByteArray::readFuint64(){
    XX(uint32_t);
}

#undef XX


int32_t ByteArray::readInt32(){
    return DecodeZigzag32(readUint32());
}


// example dec:300 bin: 0000 0010 0010 1100 
uint32_t ByteArray::readUint32(){
    uint32_t result = 0;
    for(int i = 0; i < 32; i+= 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            result |= ((uint32_t)(b & 0x7f) << i);
        }
    }
    return result;
}

int64_t ByteArray::readInt64(){
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64(){
    uint64_t result = 0;
    for(int i = 0;i < 64;i+=7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= ((uint64_t)(b & 0x7f) << i);
        }
    }
    return result;
}

float ByteArray::readFloat(){
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble(){
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

//length:int16, data
std::string ByteArray::readStringF16(){
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

//length:int32, data
std::string ByteArray::readStringF32(){
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

//length:int64, data
std::string ByteArray::readStringF64(){
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

//length:varint, data
std::string ByteArray::readStringVint(){
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

//内部操作
void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}



void ByteArray::write(const void* buf, size_t size) {
    //如果用户输入大小为0,则直接返回
    if(size == 0) {
        return;
    }
    //确保有足够的空间写入size大小的数据
    addCapacity(size);
    // 判断写指针在当前节点的偏移量
    size_t npos = m_position % m_baseSize;
    // 当前节点的可用空间
    size_t ncap = m_cur->size - npos;
    // bpos是buf缓冲区中当前写入数据的偏移量
    size_t bpos = 0;


    while(size > 0) {
        if(ncap >= size) {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            // 如果当前写完之后刚好写满当前节点，更新节点指针指向下一个节点
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }

    if(m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::read(void* buf, size_t size) {
    if(size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }
    // m_position在本块中的偏移量
    size_t npos = m_position % m_baseSize;
    // 计算从npos开始可以读取的字节数
    size_t ncap = m_cur->size - npos;
    // 写入外部buf的偏移量
    size_t bpos = 0;

    //循环读取数据
    while(size > 0) {
        //如果可以读取的字节数大于请求字节数
        if(ncap >= size) {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        }else{ //如果可以读取的字节数少于请求字节数
            memcpy((char*)buf + bpos,m_cur->ptr,ncap);
            // 更新请求字节数
            size -= ncap;
            // 更新指针指向的节点
            m_cur = m_cur->next;
            // 更新ncap的值
            ncap = m_cur->size;
            //更新操作数
            m_position += ncap;
            // 更新在当前节点的偏移量
            npos = 0;
            // 更新写入外部buf的偏移量
            bpos += ncap;
        }
    }
}


// CODING BUG
void ByteArray::read(void* buf, size_t size, size_t position) const {
    if(size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }
    // m_position在本块中的偏移量
    size_t npos = m_position % m_baseSize;
    // 计算从npos开始可以读取的字节数
    size_t ncap = m_cur->size - npos;
    // 写入外部buf的偏移量
    size_t bpos = 0;
    // 当前读数据所在的 Node
    Node* cur = m_cur;
    //循环读取数据
    while(size > 0) {
        //如果可以读取的字节数大于请求字节数
        if(ncap >= size) {
            memcpy((char*)buf + bpos, cur->ptr + npos, size);
            if(cur->size == (npos + size)) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        }else{ //如果可以读取的字节数少于请求字节数
            memcpy((char*)buf + bpos,cur->ptr + npos,ncap);
            // 更新请求字节数
            size -= ncap;
            // 更新指针指向的节点
            cur = cur->next;
            // 更新ncap的值
            ncap = cur->size;
            //更新操作数
            position += ncap;
            // 更新在当前节点的偏移量
            npos = 0;
            // 更新写入外部buf的偏移量
            bpos += ncap;
        }
    }
}

void ByteArray::addCapacity(size_t size) {
    if(size == 0) {
        return;
    }
    size_t old_cap = getCapacity();
    //如果剩余容量大于传入数据大小,直接返回
    if(old_cap >= size) {
        return;
    }
    //计算需要扩展的容量
    size = size - old_cap;
    //计算需要分配多少个node节点
    size_t count = ceil(1.0*size / m_baseSize);

    Node* tmp = m_root;
    //定位到链表末尾节点
    while(tmp->next) {
        tmp = tmp->next;
    }

    Node* first = nullptr;
    for(size_t i = 0;i < count;++i) {
        tmp->next = new Node(m_baseSize);
        if(first == nullptr) {
            first = tmp->next;
        }
        tmp = tmp->next;
        //更新总容量
        m_capacity += m_baseSize;
    }

    //如果之前完全没有剩余容量，立马更新为新添加的第一个节点
    if(old_cap == 0){
        m_cur = first;
    }
}

void ByteArray::setPosition(size_t v) {
    //边界检查
    if (v > m_capacity) {
        throw std::out_of_range("set_postion out of range");
    }
    //设置逻辑位置
    m_position = v;
    // 可能更新m_size
    if(m_position > m_size) {
        m_size = m_position;
    }
    // 从头开始定位当前块节点
    m_cur = m_root;
    while(v > m_cur->size) {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    // 如果正好为块末尾,则前移一块
    if(v ==  m_cur->size) {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile(const std::string& name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    // 判断是否打开失败
    if(!ofs) {
        DAG_LOG_ERROR(g_logger) << "writeToFile name=" << name
            << " error , error=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    // 初始化写入所需变量
    int64_t read_size = getReadSize();  // 获取当前还未读取的字节数
    int64_t pos = m_position; //从当前读取位置开始
    Node* cur = m_cur;   // 当前节点

    while(read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size);
        ofs.write(cur->ptr + diff,  len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}


bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name.c_str(),std::ios::binary);
    if(!ifs) {
        DAG_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, errno=" << errno << " errstr=" << strerror(errno);
    }
    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr) {
        delete[] ptr;
    });
    while(!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}



std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}


std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;
    for(size_t i = 0;i < str.size();++i) {
        if(i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex 
            << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    // 限制长度不可以超过实际可读部分
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) {
        return 0;
    }
    // 存储实际读取的长度
    uint64_t size = len;
    // 在当前节点内的偏移量
    size_t npos = m_position % m_baseSize;
    // 当前块还剩下多少可读取的数据容量
    size_t ncap = m_cur->size - npos;

    struct iovec iov;
    Node* cur = m_cur;

    while(len > 0) {
        // 当前剩余容量足够容纳本次全部的读取
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        }else {
            //一次读取不完
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len-=ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if(len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;
    size_t npos = m_position % m_baseSize;
    // 可以使用的容量
    size_t ncap = m_cur->size - npos;

    struct iovec iov;
    Node* cur = m_cur;

    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

}
#endif
