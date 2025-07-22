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

    TcpServer(dag::IOManager* worker = dag::IOManager::GetThis()
              ,dag::IOManager* io_worker = dag::IOManager::GetThis()
              ,dag::IOManager* accept_worker = dag::IOManager::GetThis());

    
    virtual ~TcpServer();

    virtual bool bind(dag::Address::ptr addr);

    virtual bool bind(const std::vector<Address::ptr>& addrs
                        , std::vector<Address::ptr>& fails);

    virtual bool start();

    virtual void stop();

    void setRecvTimellout(uint64_t v) { m_recvTimeout = v; }

    uint64_t getRecvTimeout() const { return m_recvTimeout; }

    void setName(const std::string& v) { m_name = v;}

    std::string getName() const { return m_name;}

    bool isStop() const { return m_isStop;};

    virtual std::string toString(const std::string& prefix = "");

    std::vector<Socket::ptr> getSocks() const { return m_socks;}

protected:
    virtual void handleClient(Socket::ptr client);

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
