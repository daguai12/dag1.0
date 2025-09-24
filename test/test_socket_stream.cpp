#include "address.h"
#include "bytearray.h"
#include "logger.h"
#include "ioscheduler.h"
#include "socket_stream.h"
#include "socket.h"
#include "tcp_server.h"
#include <cstring>
#include <memory>

using namespace dag;

Logger::ptr g_logger = DAG_LOG_ROOT();

class EchoServer : public TcpServer {
public:
    using ptr = std::shared_ptr<EchoServer>;

    EchoServer(IOManager* worker, IOManager* io_worker, IOManager* accept_worker)
        :TcpServer(worker, io_worker, accept_worker) {}

protected:
    void handleClient(Socket::ptr client) override {
        DAG_LOG_INFO(g_logger) << "New client: " << client->getRemoteAddress()->toString();

        SocketStream::ptr  stream = std::make_shared<SocketStream>(client);
        ByteArray::ptr ba(new ByteArray);

        while(true) {
            ba->clear();
            ba->setPosition(0);

            int rt = stream->read(ba, 1024);
            if (rt == 0) {
                DAG_LOG_INFO(g_logger) << "Client closed: " << stream->getRemoteAddress();
                break;
            } else if (rt < 0) {
                DAG_LOG_ERROR(g_logger) << "recv error: " << strerror(errno);
                break;
            }

            ba->setPosition(0);
            std::string data = ba->toString();
            DAG_LOG_INFO(g_logger) << "recv from client" << data;

            ba->setPosition(0);
            stream->write(ba, rt);
        }
        stream->close();
    }
};

void run_server() {
    IOManager* worker = IOManager::GetThis();
    EchoServer::ptr server = std::make_shared<EchoServer>(worker, worker, worker);

    Address::ptr addr = Address::LookupAny("0.0.0.0:8033");
    if(!server->bind(addr)) {
        DAG_LOG_ERROR(g_logger) << "Bind failed";
        return;
    }

    server->start();
    DAG_LOG_INFO(g_logger) << "EchoServer started at:" << addr->toString();

}

int main()
{
    IOManager iom(2);
    iom.schedulerLock(run_server);
    return 0;
}
