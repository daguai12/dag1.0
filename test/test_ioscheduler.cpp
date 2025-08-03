#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "ioscheduler.h"

dag::IOManager iom(2);

const char* str = "Hello, World!";

void setNonBlock(int listenfd) {
    int flags = fcntl(listenfd, F_GETFL);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);
}

void handle_client(int client_fd) {
    char buffer[1024] = {0};
    int ret = recv(client_fd, buffer, sizeof(buffer), 0);
    if (ret <= 0) {
        close(client_fd);
        return;
    }
    std::cout << "Received: " << buffer << std::endl;

    const char* reply = "HTTP/1.1 200OK\r\nContent-length: 13\r\n\nHello,World!";
    send(client_fd,reply,  strlen(reply),  0);

    iom.addEvent(client_fd, dag::IOManager::READ,[client_fd](){
        handle_client(client_fd);
    });
}

void acceptLoop(int listenfd) {
    while (true) {
        int client_fd = accept(listenfd, nullptr, 0);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            break;
        }

        std::cout << "New connection: " << client_fd << std::endl;
        setNonBlock(client_fd);

        iom.addEvent(client_fd, dag::IOManager::READ,[client_fd](){
            handle_client(client_fd);
        });
    }

    iom.addEvent(listenfd, dag::IOManager::READ,[listenfd](){
        acceptLoop(listenfd);
    });
}

int main()
{
    int port = 8083;
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if (listen_fd < 0) {
        std::cout <<  "socket" << "error" << std::endl;
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    int ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    if (ret < 0) {
        std::cout << "setsockopt" << "error" << std::endl;
    }


    ret = bind(listen_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (ret < 0) {
        std::cout  << "bind" << "error" << std::endl;
    }

    ret = listen(listen_fd, SOMAXCONN);

    std::cout << "Server listening on port: " << port << std::endl;
    setNonBlock(listen_fd);

    iom.addEvent(listen_fd, dag::IOManager::READ,[listen_fd](){
        acceptLoop(listen_fd);
    });
    



    return 0;
}

