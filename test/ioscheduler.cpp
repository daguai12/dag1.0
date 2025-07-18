#include "ioscheduler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <cstring>
#include <cerrno>

using namespace std;

char recv_data[4096];

const char send_data[] = "GET / HTTP/1.0\r\n";

int sock;

void func()
{
    recv(sock, recv_data, 4096, 0);
    std::cout << recv_data << std::endl << std::endl;;
}

void func2()
{
    send(sock, send_data, sizeof(send_data), 0);
}

int main()
{
    dag::IOManager manager(2);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = inet_addr("192.168.101.128");

    fcntl(sock, F_SETFL,O_NONBLOCK);
    
    connect(sock, (struct sockaddr *)&server, sizeof(server));

    manager.addEvent(sock, dag::IOManager::WRITE,&func2);
    manager.addEvent(sock, dag::IOManager::READ,&func);

    std::cout << "event has benn posted\n\n";
    
    return 0;
}
