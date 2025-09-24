#ifndef _DAG_SOCKET_H_
#define _DAG_SOCKET_H_

#include <bits/types/struct_iovec.h>
#include <cstdint>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include "address.h"
#include "./utils/noncopyable.h"

namespace dag {

/**
* @brief 套接字分装
*/
class Socket : public std::enable_shared_from_this<Socket>, NonCopyable {
public:
    using ptr = std::shared_ptr<Socket>;
    using weak_ptr = std::weak_ptr<Socket>;

    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM 
    };

    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };

    /**
    * @brief 根据地址类型创建TCP Socket
    * @param[in] address 地址对象
    * @return Socket::ptr 成功返回Socket智能指针，失败返回nullptr
    */
    static Socket::ptr CreateTCP(dag::Address::ptr address);

    /**
    * @brief 根据地址类型创建UDP Socket
    * @param[in] address 地址对象
    * @return Socket::ptr 成功返回Socket智能指针，失败返回nullptr
    */
    static Socket::ptr CreateUDP(dag::Address::ptr address);

    /**
    * @brief 创建一个IPv4的TCP Socket
    * @return Socket::ptr 成功返回Socket智能指针，失败返回nullptr
    */
    static Socket::ptr CreateTCPSocket();

    /**
    * @brief 创建一个IPv4的TCP Socket
    * @return Socket::ptr 成功返回Socket智能指针，失败返回nullptr
    */
    static Socket::ptr CreateUDPSocket();

    /**
    * @brief Socket构造函数
    * @param[in] family 地址簇
    * @param[in] type 套结字类型
    * @param[in] protocol 协议（通常为0)
    */
    Socket(int family, int type, int protocol = 0);

    /**
    * @brief 析构函数，会自动关闭socket
    */
    virtual ~Socket();

    /**
    * @brief 获取发送超时时间（毫秒）
    * @return int64_t 超时时间
    * @note 此处超时由框架中FdManager管理，而非内核的SO_SNDTIMEO
    */
    int64_t getSendTimeout();

    /**
    * @brief 设置发送超时时间
    * @param[in] v 超时时间（毫秒)
    */
    void SetSendTimeout(int64_t v);

    /**
    * @brief 获取接受超时时间（毫秒）
    * @return int64_t 超时时间
    * @note 此处超时由框架中FdManager管理,而非内核的SO_RCVTIMEO
    */
    int64_t getRecvTimeout();

    /**
    * @brief 设置接受超时时间
    * @param[in] v 超时时间（毫秒)
    */
    void setRecvTimeout(int64_t v);

    /**
    * @brief 获取Socket选项
    * @param[in] level 协议层
    * @param[in] option 选项名称
    * @param[out] result 存放结果的缓冲区
    * @param[int, out] len result缓冲区大小
    * @return bool 是否成功
    */
    bool getOption(int level, int option, void* result, socklen_t* len);

    /**
     * @brief 获取Socket选项
     * @tparam T 选项值的类型
     * @param[in] level 协议层
     * @param[in] option 选项名称
     * @param[out] result 存放结果的变量
     * @return bool 是否成功
     */
    template<class T>
    bool getOption(int level, int option, T& result) {
        size_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    
     /**
     * @brief 设置Socket选项 
     * @param[in] level 协议层
     * @param[in] option 选项名称
     * @param[in] result 包含新值的缓冲区
     * @param[in] len result缓冲区的大小
     * @return bool 是否成功
     */
    bool setOption(int level, int option, const void* result, size_t len);

     /**
     * @brief 设置Socket选项
     * @tparam T 选项值的类型
     * @param[in] level 协议层
     * @param[in] option 选项名称
     * @param[in] value 要设置的值
     * @return bool 是否成功
     */
    template<class T>
    bool setOption(int level, int option,const T& value){
        return setOption(level, option, &value, sizeof(T));
    }

    /**
     * @brief 接受一个新连接 (仅用于监听socket)
     * @return Socket::ptr 代表新连接的Socket，失败返回nullptr
     */
    Socket::ptr accept();

    /**
     * @brief 绑定地址
     * @param[in] addr 要绑定的本地地址
     * @return bool 是否成功
     */
    bool bind(const Address::ptr addr);


    /**
    * @brief 连接到远程地址
    * @param[in] addr 要连接的远程地址
    * @param[in] timeout_ms 连接超时时间(毫秒)。-1表示使用系统默认阻塞行为。
    * @return bool 是否成功
    */
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    /**
    * @brief 将socket置为监听模式
    * @param[in] backlog 等待连接队列的最大长度
    * @return bool 是否成功
    */
    bool listen(int backlog = SOMAXCONN);

    /**
     * @brief 关闭socket
     * @return bool 是否成功
     */
    bool close();

    /**
     * @brief 发送数据 (用于TCP)
     * @param[in] buffer 数据缓冲区
     * @param[in] length 数据长度
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 发送的字节数, =0 连接关闭, <0 出错
     */
    int send(const void* buffer,size_t length, int flags = 0);

    /**
     * @brief 发送多个数据块 (聚集发送，用于TCP)
     * @param[in] buffers iovec结构体数组
     * @param[in] length 数组长度
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 发送的字节数, =0 连接关闭, <0 出错
     */
    int send(const iovec* buffers, size_t length, int flags = 0);


    /**
     * @brief 发送数据到指定地址 (用于UDP)
     * @param[in] buffer 数据缓冲区
     * @param[in] length 数据长度
     * @param[in] to 目标地址
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 发送的字节数, <0 出错
     */
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 发送多个数据块到指定地址 (聚集发送, 用于UDP)
     * @param[in] buffer iovec结构体数组
     * @param[in] length 数组长度
     * @param[in] to 目标地址
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 发送的字节数, <0 出错
     */
    int sendTo(const iovec* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 接收数据 (用于TCP)
     * @param[out] buffer 数据缓冲区
     * @param[in] length 缓冲区长度
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 接收的字节数, =0 连接关闭, <0 出错
     */
    int recv(void* buffer, size_t length, int flags = 0);

     /**
     * @brief 接收数据到多个缓冲区 (分散接收, 用于TCP)
     * @param[out] buffer iovec结构体数组
     * @param[in] length 数组长度
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 接收的字节数, =0 连接关闭, <0 出错
     */
    int recv(iovec* buffer, size_t length, int flags = 0);

    /**
     * @brief 从socket接收数据并获取源地址 (用于UDP)
     * @param[out] buffer 数据缓冲区
     * @param[in] length 缓冲区长度
     * @param[out] from 源地址对象
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 接收的字节数, <0 出错
     */
    int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);

     /**
     * @brief 从socket接收数据到多个缓冲区并获取源地址 (分散接收, 用于UDP)
     * @param[out] buffer iovec结构体数组
     * @param[in] length 数组长度
     * @param[out] from 源地址对象
     * @param[in] flags 标志位 (默认为0)
     * @return int >0 接收的字节数, <0 出错
     */
    int recvFrom(iovec* buffer, size_t length, Address::ptr from, int flags = 0);

     /**
     * @brief 获取远端地址
     * @return Address::ptr 地址对象智能指针
     */
    Address::ptr getRemoteAddress();

    /**
     * @brief 获取本地地址
     * @return Address::ptr 地址对象智能指针
     */
    Address::ptr getLocalAddress();

    /**
     * @brief 获取地址簇
     * @return int 地址簇
     */
    int getFamily() const { return m_family;}

    /**
     * @brief 获取套接字类型
     * @return int 套接字类型
     */
    int getType() const {return m_type;}

     /**
     * @brief 获取协议类型
     * @return int 协议类型
     */
    int getProtocol() const {return m_protocol;}

    /**
     * @brief 检查socket是否已连接
     * @return bool
     */
    bool isConnected() const {return m_isConnected;};

     /**
     * @brief 检查socket是否有效 (文件描述符是否存在)
     * @return bool
     */
    bool isValid() const;

    /**
     * @brief 获取socket上的错误码
     * @return int 错误码
     * @note 调用此函数会清除socket上的错误状态
     */
    int getError();

    /**
     * @brief 将socket信息输出到流
     * @param[in, out] os 输出流
     * @return std::ostream& 输出流引用
     */
    virtual std::ostream& dump(std::ostream& os) const;

    /**
     * @brief 将socket信息转换为字符串
     * @return std::string
     */
    virtual std::string toString() const;

    /**
     * @brief 获取socket文件描述符
     * @return int
     */
    int getSocket() const {return m_sock;};

    /**
     * @brief (异步)取消读事件
     * @return bool 是否成功提交取消请求
     * @note 仅在异步IO调度器中使用
     */
    bool cancelRead();

     /**
     * @brief (异步)取消写事件
     * @return bool 是否成功提交取消请求
     * @note 仅在异步IO调度器中使用
     */
    bool cancelWrite();

    /**
     * @brief (异步)取消accept事件
     * @return bool 是否成功提交取消请求
     * @note 仅在异步IO调度器中使用
     */
    bool cancelAccept();

     /**
     * @brief (异步)取消所有事件
     * @return bool 是否成功提交取消请求
     * @note 仅在异步IO调度器中使用
     */
    bool cancelAll();
private:
    /**
    * @brief 初始化socket的默认选项 (SO_REUSEADDR, TCP_NODELAY)
    */
    void initSock();

    /**
     * @brief 创建新的socket文件描述符
     */
    void newSock();

    /**
     * @brief 使用一个已存在的文件描述符初始化Socket对象
     * @param[in] sock 已存在的文件描述符
     * @return bool 是否初始化成功
     */
    bool init(int sock);
protected:
    /// @brief 套接字句柄(文件描述符)
    int m_sock;
    /// @brief 地址簇(AF_INET, AF_UNIX)
    int m_family;
    /// @brief 套接字类型(SOCK_STREAM, SOCK_DGRAM)
    int m_type;
    /// @brief 协议类型(通常为0)
    int m_protocol;
    /// @brief 是否已链接的标志
    int m_isConnected;

    /// @brief 本地地址信息对象
    Address::ptr m_localAddress;
    /// @brief 远端地址信息对象
    Address::ptr m_remoteAddress;
};

std::ostream& operator<<(std::ostream& os, const Socket& sock);

}

#endif
