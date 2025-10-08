#ifndef _DAG_TCP_SERVER_H_
#define _DAG_TCP_SERVER_H_

#include "ioscheduler.h"
#include "utils/noncopyable.h"
#include "address.h"
#include "socket.h"
#include <memory>


namespace  dag {

/**
* @brief TCP服务器封装
*/
class TcpServer : public std::enable_shared_from_this<TcpServer>, NonCopyable {
public:
    using ptr = std::shared_ptr<TcpServer>;

    /**
    * @brief 构造函数
    * @param[in] worker socket客户端工作的协程调度器
    * @param[in] accept_worker 服务器socket执行接受socket连接的协程调度器
    */
    TcpServer(dag::IOManager* worker = dag::IOManager::GetThis()
              ,dag::IOManager* io_worker = dag::IOManager::GetThis()
              ,dag::IOManager* accept_worker = dag::IOManager::GetThis());

    /**
    * @brief 析构函数
    */
    virtual ~TcpServer();

    /**
    * @brief 绑定地址
    * @return 返回是否绑定成功
    */
    virtual bool bind(dag::Address::ptr addr);

    /**
    * @brief 绑定地址数组
    * @param[in] addrs 需要绑定的地址数组
    * @param[out] fails 绑定失败的地址
    */
    virtual bool bind(const std::vector<Address::ptr>& addrs
                        , std::vector<Address::ptr>& fails);

    /**
    * @brief 启动服务
    */
    virtual bool start();

    /**
    * @brief 停止服务
    */
    virtual void stop();

    /**
    * @brief 设置读取超时时间
    */
    void setRecvTimellout(uint64_t v) { m_recvTimeout = v; }

    /**
    * @brief 返回读取超时时间
    */
    uint64_t getRecvTimeout() const { return m_recvTimeout; }

    /**
    * @brief 设置服务器名称
    */
    void setName(const std::string& v) { m_name = v;}

    /**
    * @brief 返回服务器名称
    */
    std::string getName() const { return m_name;}

    /**
    * @brief 是否停止
    */
    bool isStop() const { return m_isStop;};

    virtual std::string toString(const std::string& prefix = "");

    std::vector<Socket::ptr> getSocks() const { return m_socks;}

protected:
    /**
    * @brief 处理新连接的socket类
    */
    virtual void handleClient(Socket::ptr client);

    /**
    * @brief 开始接受连接
    */
    virtual void startAccept(Socket::ptr sock);

protected:
    // 监听Socket数组
    std::vector<Socket::ptr> m_socks;
    // 新链接的Socket工作调度器
    IOManager* m_worker;
    IOManager* m_ioWorker;
    // 服务器Socket接受连接的调度器
    IOManager* m_acceptWorker;
    // 接收超时时间（毫秒)
    uint64_t m_recvTimeout;
    // 服务器名称
    std::string m_name;
    // 服务器类型
    std::string m_type = "tcp";
    // 服务是否停止
    bool m_isStop;
};


}


#endif
