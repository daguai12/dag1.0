#include "address.h"
#include "socket.h"
#include "logger.h"
#include "ioscheduler.h"
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace dag;

static dag::Logger::ptr g_logger = DAG_LOG_ROOT();

bool test_socket() {
    struct sockaddr_in sock1;
    sock1.sin_family = AF_INET;
    sock1.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.70.239",&sock1.sin_addr.s_addr);

    IPAddress::ptr addr(new IPv4Address(sock1));

    if(addr) {
        DAG_LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        DAG_LOG_ERROR(g_logger) << "get address fail";
        return false;
    }

    addr->setPort(80);
    dag::Socket::ptr sock = dag::Socket::CreateTCP(addr);

    if(!sock->connect(addr)) {
        DAG_LOG_ERROR(g_logger) << "connect" << addr->toString() << " fail";
    } else {
        DAG_LOG_INFO(g_logger) << "coonect" << addr->toString() << "connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        DAG_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return  true;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    std::cout << buffs << std::endl;

    while(1) {
        sleep(1);
    }

    if(rt <= 0) {
        DAG_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return true;
    }
    return true;
}

int main() {
    dag::IOManager iom;
    iom.schedulerLock(&test_socket);
    return 0;

}
