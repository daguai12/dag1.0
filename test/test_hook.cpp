#include "ioscheduler.h"
#include "hook.h"
#include "logger.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

static dag::Logger::ptr g_logger = DAG_LOG_ROOT();

dag::IOManager iom(6,true,"dag_server");

int socket_init() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        DAG_LOG_ERROR(g_logger) << "socket error";
    }

    int port = 8888;
    sockaddr_in server_addr;
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        DAG_LOG_ERROR(g_logger) << "setsockopt";
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr)) == -1) {
        DAG_LOG_ERROR(g_logger) << "bind error";
    }

    if (listen(sockfd, SOMAXCONN) == -1) {
        DAG_LOG_ERROR(g_logger) << "listen error";
    }

    return sockfd;
}

void handle_client(int clientfd) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    int n = recv(clientfd, buf, sizeof(buf), 0);

    if (n == 0) {
        DAG_LOG_INFO(g_logger) << "client closed connection, fd = " << clientfd;
        iom.delEvent(clientfd, dag::IOManager::READ);
        close(clientfd);
        return;
    }

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        if (errno == ETIMEDOUT) {
            const char* str =  "timeout, please reconnected!";
            send(clientfd,str,strlen(str),0);
            return;
        }
        DAG_LOG_ERROR(g_logger) << "recv error";
        iom.delEvent(clientfd, dag::IOManager::READ);
        close(clientfd);
        return;
    }

    DAG_LOG_INFO(g_logger) << "received: " << buf;

    
    n = send(clientfd, buf, strlen(buf), 0);

    if (n < 0) {
        DAG_LOG_ERROR(g_logger) << "send error";
        close(clientfd);
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
        DAG_LOG_ERROR(g_logger) << "setsockopt error";
        exit(EXIT_FAILURE);
    } else {
        DAG_LOG_INFO(g_logger) << "setsockopt successfule";
    }

    dag::IOManager::GetThis()->addEvent(clientfd, dag::IOManager::READ,[clientfd](){
        handle_client(clientfd);
    });
}

void acceptLoop(int sockfd) {
    
    while (true) {
        sockaddr client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);

        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        int clientfd = accept(sockfd, &client_addr, &client_addr_len);
        


        if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
            DAG_LOG_ERROR(g_logger) << "setsockopt error";
            exit(EXIT_FAILURE);
        } else {
            DAG_LOG_INFO(g_logger) << "setsockopt successful";
        }

        DAG_LOG_INFO(g_logger) << "fiber is running";


        if (clientfd > 0) {
            DAG_LOG_INFO(g_logger) << "clientfd: " << clientfd;

            char ip_str[INET6_ADDRSTRLEN];
            uint16_t port;

            sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(&client_addr);
            inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);
            port = ntohs(addr_in->sin_port);

            DAG_LOG_INFO(g_logger) << "客户端地址: " << ip_str << ":" << port;
        } else {
            DAG_LOG_ERROR(g_logger) << "accept error";
            break;
        }

        dag::IOManager::GetThis()->addEvent(clientfd, dag::IOManager::READ,[clientfd](){
            handle_client(clientfd);
        });
    }

    iom.addEvent(sockfd, dag::IOManager::READ,[sockfd](){
        acceptLoop(sockfd);
    });
}

int main()
{

    int sockfd = socket_init();

    dag::set_hook_enable(true);
    iom.addEvent(sockfd, dag::IOManager::READ,[sockfd](){
        acceptLoop(sockfd);
    });


    return 0;
}
