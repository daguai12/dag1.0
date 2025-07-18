#if 0
#ifndef _DAG_ADDRESS_H_
#define _DAG_ADDRESS_H_

#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
#include <map>
#include <vector>
#include <arpa/inet.h>

namespace dag {

class IPAddress;

/**
 * @brief 网络地址的基类，抽象类
 */
class Address {
public:
    using ptr = std::shared_ptr<Address>;

    /**
    * @brief 通过 sockaddr *创建对应的 Address
    * @param addr sockaddr 地址
    * @return Address::ptr
    */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    /**
     * @brief 通过host地址返回对应条件的所有Address
     * @param[out] result 保存满足条件的Address
     * @param[in] host 域名,服务器名等.举例: www.baidu.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
                      int family = AF_INET, int type = 0, int protocol = 0);


    /**
     * @brief 通过host地址返回对应条件的任意Address
     * @param[in] host 域名,服务器名等.举例: www.baidu.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意Address,失败返回nullptr
     */
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);


     /**
     * @brief 通过host地址返回对应条件的任意IPAddress
     * @param[in] host 域名,服务器名等.举例: www.baidu.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意IPAddress,失败返回nullptr
     */
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
     * @param[out] result 保存本机所有地址
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    /**
     * @brief 获取指定网卡的地址和子网掩码位数
     * @param[out] result 保存指定网卡所有地址
     * @param[in] iface 网卡名称
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);

    /**
    * @brief 默认构造函数
    */
    Address() = default;

    /**
    * @brief 虚析构函数
    */
    virtual ~Address() = default;

    /**
    * @brief 获取原始地址，只读
    * @return sockaddr 地址
    */
    virtual const sockaddr* getAddr() const = 0;

    /**
     * @brief 获取原始地址，可写
     * @return sockaddr地址
     */
    virtual sockaddr* getAddr()  = 0;
     

    /**
    * @brief 获取原始地址的长度
    */
    virtual socklen_t getAddrLen() const = 0;

    /**
    * @brief 获取地址的协议族
    */
    int getFamily() const;

    /**
    * @brief 将数据写入 ostream, 配合 toString
    * @param os 输出流
    * @return 输出流
    */
    virtual std::ostream &insert(std::ostream& os) const = 0;


    /**
    * @brief 将地址格式转化为字符串
    * @return 字符串
    */
    std::string toString() const;

};

/**
* @brief IP地址类，抽象类
*/
class IPAddress : public Address {
public:
    using ptr = std::shared_ptr<IPAddress>;

    /**
     * @brief 通过域名,IP,服务器名创建IPAddress
     * @param[in] address 域名,IP,服务器名等.举例: www.sylar.top
     * @param[in] port 端口号
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    
    /**
    * @brief 获取端口号
    * @return 端口号
    */
    virtual uint16_t getPort() const = 0;

    /**
    * @brief 设置端口号
    */
    virtual void setPort(uint16_t v) = 0; 
};


/**
* @brief IPv4地址
*/
class IPv4Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv4Address>;

    /**
    * @brief 通过地址和端口号创建对应的 IPv4Address
    * @param address 点分十进制地址字符串
    * @param port 端口号
    * @return IPv4Address
    */
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);
    
    /**
    * @brief 构造函数
    * @param address sockaddr_in 地址
    */
    explicit IPv4Address(const sockaddr_in &address);

    /**
    * @brief 构造函数
    * @param address 32位地址
    * @param port 端口号
    */
    explicit IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    /**
    * @brief 析构函数
    */
    ~IPv4Address() override = default;

    /**
    * @brief 获取原始地址，只读
    * @return sockaddr 地址
    */
    const sockaddr* getAddr() const override;

    /**
    * @brief 获取原始地址，可写
    * @return sockaddr 地址
    */
    sockaddr *getAddr() override;

    /**
     * @brief 获取原始地址的长度
     */
    socklen_t getAddrLen() const override;

    /**
    * @brief 将数据写入 ostream,配合 toString
    * @param os输出流
    * @return 输出流
    */
    std::ostream& insert(std::ostream& os) const override;

    /**
    * @brief 获取端口号
    * @return 端口号
    */
    uint16_t getPort() const override;

    void setPort(uint16_t v) override;

private:
    // IPv4 原始地址
    sockaddr_in m_addr;
};

/**
* @brief 未知地址
*/
class UnknownAddress : public Address {
public:
    using ptr = std::shared_ptr<UnknownAddress>;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;

};

/**
* @brief 流地址输出
*/
std::ostream& operator<<(std::ostream& os, const Address& addr);

}

#endif


#endif




#ifndef __DAG_ADDRESS_H__
#define __DAG_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

namespace dag {

class IPAddress;

/**
 * @brief 网络地址的基类,抽象类
 */
class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    /**
     * @brief 通过sockaddr指针创建Address
     * @param[in] addr sockaddr指针
     * @param[in] addrlen sockaddr的长度
     * @return 返回和sockaddr相匹配的Address,失败返回nullptr
     */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    /**
     * @brief 通过host地址返回对应条件的所有Address
     * @param[out] result 保存满足条件的Address
     * @param[in] host 域名,服务器名等.举例: www.dag.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    /**
     * @brief 通过host地址返回对应条件的任意Address
     * @param[in] host 域名,服务器名等.举例: www.dag.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意Address,失败返回nullptr
     */
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    /**
     * @brief 通过host地址返回对应条件的任意IPAddress
     * @param[in] host 域名,服务器名等.举例: www.dag.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意IPAddress,失败返回nullptr
     */
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
     * @param[out] result 保存本机所有地址
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    /**
     * @brief 获取指定网卡的地址和子网掩码位数
     * @param[out] result 保存指定网卡所有地址
     * @param[in] iface 网卡名称
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);

    /**
     * @brief 虚析构函数
     */
    virtual ~Address() {}

    /**
     * @brief 返回协议簇
     */
    int getFamily() const;

    /**
     * @brief 返回sockaddr指针,只读
     */
    virtual const sockaddr* getAddr() const = 0;

    /**
     * @brief 返回sockaddr指针,读写
     */
    virtual sockaddr* getAddr() = 0;

    /**
     * @brief 返回sockaddr的长度
     */
    virtual socklen_t getAddrLen() const = 0;

    /**
     * @brief 可读性输出地址
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;

    /**
     * @brief 返回可读性字符串
     */
    std::string toString() const;

    /**
     * @brief 小于号比较函数
     */
    bool operator<(const Address& rhs) const;

    /**
     * @brief 等于函数
     */
    bool operator==(const Address& rhs) const;

    /**
     * @brief 不等于函数
     */
    bool operator!=(const Address& rhs) const;
};

/**
 * @brief IP地址的基类
 */
class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    /**
     * @brief 通过域名,IP,服务器名创建IPAddress
     * @param[in] address 域名,IP,服务器名等.举例: www.dag.top
     * @param[in] port 端口号
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    /**
     * @brief 获取该地址的广播地址
     * @param[in] prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

    /**
     * @brief 获取该地址的网段
     * @param[in] prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;

    /**
     * @brief 获取子网掩码地址
     * @param[in] prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    /**
     * @brief 返回端口号
     */
    virtual uint32_t getPort() const = 0;

    /**
     * @brief 设置端口号
     */
    virtual void setPort(uint16_t v) = 0;
};

/**
 * @brief IPv4地址
 */
class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    /**
     * @brief 使用点分十进制地址创建IPv4Address
     * @param[in] address 点分十进制地址,如:192.168.1.1
     * @param[in] port 端口号
     * @return 返回IPv4Address,失败返回nullptr
     */
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

    /**
     * @brief 通过sockaddr_in构造IPv4Address
     * @param[in] address sockaddr_in结构体
     */
    IPv4Address(const sockaddr_in& address);

    /**
     * @brief 通过二进制地址构造IPv4Address
     * @param[in] address 二进制地址address
     * @param[in] port 端口号
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};



/**
 * @brief 未知地址
 */
class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

/**
 * @brief 流式输出Address
 */
std::ostream& operator<<(std::ostream& os, const Address& addr);

}

#endif

