#include "../dag/stream/socket_stream.h"
#include "ioscheduler.h"
#include "logger.h"
#include "address.h"



using namespace dag;

static Logger::ptr g_logger = DAG_LOG_ROOT();

void run() {
    auto addr = Address::LookupAnyIPAddress("127.0.0.1:8033");
    Socket::ptr sock = Socket::CreateTCP(addr);
    if (!sock->connect(addr)) {
        DAG_LOG_ERROR(g_logger) << "connect to " << addr->toString() << " failed";
        return;
    }

    DAG_LOG_INFO(g_logger) << "Connected to " << addr->toString();

    SocketStream::ptr stream(new SocketStream(sock));
    std::string line;

    while (std::getline(std::cin, line)) {
        stream->write(line.c_str(), line.size());

        char buffer[1024] = {0};
        int rt = stream->read(buffer, sizeof(buffer) - 1);
        if (rt <= 0) {
            DAG_LOG_INFO(g_logger) << "Server closed connection.";
            break;
        }

        std::cout << "Echo: " << std::string(buffer, rt) << std::endl;
    }

    stream->close();
}

int main() {
    IOManager iom(1);
    iom.schedulerLock(run);
    return 0;
}
