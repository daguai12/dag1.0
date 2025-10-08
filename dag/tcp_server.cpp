#include "tcp_server.h"
#include "ioscheduler.h"
#include "logger.h"
#include "socket.h"
#include <cstring>


namespace dag {

static dag::Logger::ptr g_logger = DAG_LOG_ROOT();

TcpServer::TcpServer(dag::IOManager* worker,
                     dag::IOManager* io_worker,
                     dag::IOManager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(50000) //DoCoding
    ,m_name("dag/1.0")
    ,m_isStop(true)
{

}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(dag::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}


bool TcpServer::bind(const std::vector<Address::ptr>& addrs
                     , std::vector<Address::ptr>& fails)
{
    // 遍历每一个地址并尝试绑定监听
    for(auto& addr : addrs) {
        // 创建一个socket套接字
        Socket::ptr sock = Socket::CreateTCPSocket();
        // 尝试绑定地址
        if(!sock->bind(addr)) {
            #if DEBUG
            DAG_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            #endif
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            #if DEBUG
            DAG_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            #endif
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }

    // 如果有绑定失败的socket,则将所有socket都清除
    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        #if DEBUG
        DAG_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " server bidn success: " << *i;
        #endif
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_recvTimeout);
            m_ioWorker->schedulerLock(std::bind(&TcpServer::handleClient,
                        shared_from_this(),client));
        } else {
            #if DEBUG
            DAG_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
            #endif
        }
    }
}

bool TcpServer::start() {
    if(!m_isStop) {
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks) {
        m_acceptWorker->schedulerLock(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
    }
    return true;
}


void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedulerLock([this, self](){
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

void TcpServer::handleClient(Socket::ptr client) {
    #if DEBUG
    DAG_LOG_INFO(g_logger) << "handleClient: " << *client;
    #endif
}


std::string TcpServer::toString(const std::string& prefix) {
    std::stringstream ss;
    ss << prefix << "[type=" << m_type
       << " name=" << m_name 
       << " worker=" << (m_worker ? m_worker->getName() : "")
       << " accept=" << (m_acceptWorker ? m_acceptWorker->getName() : "")
       << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
    std::string pfx = prefix.empty() ? "    " : prefix;
    for(auto& i : m_socks) {
        ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}

}
