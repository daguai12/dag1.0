#include "address.h"
#include "ioscheduler.h"
#include "tcp_server.h"
#include "socket.h"
#include "logger.h"

using namespace dag;

dag::Logger::ptr g_logger = DAG_LOG_ROOT();

class myTcpServer : public dag::TcpServer{
public:
    myTcpServer(): dag::TcpServer() {}
    void handleClient(dag::Socket::ptr client) override {
        DAG_LOG_INFO(g_logger) << "Client connected: " << client->toString();
        char buffer[1024];

        while (true) {
            memset(buffer,0,sizeof(buffer));
            int bytes = client->recv(buffer, sizeof(buffer));
            if (bytes == 0) {
                DAG_LOG_INFO(g_logger) << "client disconnected: " << client->toString();
                break;
            } else if (bytes < 0) {
                DAG_LOG_ERROR(g_logger) << "recv error: " << strerror(errno);
                break;

            }
            client->send(buffer, bytes);
        }
    }
};



void run() {
    auto addr = dag::Address::LookupAny("0.0.0.0:8033");
    std::vector<dag::Address::ptr> addrs;
    addrs.push_back(addr);

    myTcpServer::ptr tcp_server(new myTcpServer);
    std::cout << tcp_server->getName() << std::endl;
    std::vector<dag::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

int main()
{
    dag::IOManager iom(10);
    iom.schedulerLock(run);
    return 0;
}




