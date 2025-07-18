#include "tcp_server.h"

#include <cstring>
#include "logger.h"
#include "./utils/asserts.h"

namespace dag {
TCPServer::TCPServer(std::string name, IOManager *acceptor, IOManager *worker)
    : name_(std::move(name))
    , acceptor_(acceptor)
    , worker_(worker)
    , stop_(false)
{
    DAG_LOG_INFO(DAG_LOG_ROOT()) << "create a new tcp server, name = " << getName();
}

TCPServer::~TCPServer() {
    if (!stop_) {
        stop();
    }
    DAG_LOG_INFO(DAG_LOG_ROOT()) << "tcp server stop, name = " << getName();
}

bool TCPServer::bind(const Address::ptr &address) {
    sock_ = Socket::CreateTCP(address->getFamily());
    if (!sock_->bind(address)) {
        DAG_LOG_ERROR(DAG_LOG_ROOT()) << "bind filed errno =" << errno
                                      <<  " errstr=" << strerror(errno)
                                      <<  " addr=[" << address->toString() << "";
        return false;
    }
    if (!sock_->listen()) {
        DAG_LOG_ERROR(DAG_LOG_ROOT()) << "listen filed errno=" << errno
                                             << " errstr = " << strerror(errno)
                                             << "addr=[" << address->toString() << "";
        return false;
    }
    return true;
}

void TCPServer::stop() {
    stop_ = true;
    acceptor_->scheduler([](){

    });
};




}
