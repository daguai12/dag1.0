#include "ioscheduler.h"
#include "hook.h"
#include <asm-generic/socket.h>
#include <cerrno>
#include <netinet/in.h>
#include <ratio>
#include <sys/epoll.h>
#include <type_traits>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <stack>
#include <cstring>
#include <chrono>
#include <thread>
#include <valarray>


static int sock_list_fd = -1;

void test_accept();

void error(const char* msg)
{
    perror(msg);
    printf("error...\n");
    exit(1);
}

void watch_io_read()
{
    dag::IOManager::GetThis()->addEvent(sock_list_fd, dag::IOManager::READ,test_accept);

}

void test_accept()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);


    int fd = accept(sock_list_fd, (struct sockaddr*)&addr, &len);
    if (fd < 0)
    {
        std::cout << "accept failed, fd = " << fd << ", errno = " << errno << std::endl;
    }
    else
    {
        std::cout << "accepted connection, fd = " << fd << std::endl;

        // fcntl(fd, F_SETFL, O_NONBLOCK);
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        dag::IOManager::GetThis()->addEvent(fd, dag::IOManager::READ, [fd]()
        {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            while (true)
            {
                int ret = recv(fd, buffer, sizeof(buffer), 0);
                if (ret > 0)
                {
                    // 打印接收到的数据
                    std::cout << "received data, fd = " << fd << ", data = " << buffer << std::endl;
                    
                    // 构建HTTP响应
                    const char *response = "HTTP/1.1 200 OK\r\n"
                                           "Content-Type: text/plain\r\n"
                                           "Content-Length: 13\r\n"
                                           "Connection: keep-alive\r\n"
                                           "\r\n"
                                           "Hello, World!";
                    
                    // 发送HTTP响应
                    ret = send(fd, response, strlen(response), 0);
                   std::cout << "sent data, fd = " << fd << ", ret = " << ret << std::endl;

                }
                if (ret <= 0)
                {
                    if (ret == 0 || errno != EAGAIN)
                    {
                        std::cout << "ret对应的返回值是: " << ret << std::endl;
                        std::cout << "errnor对应的值是多少" << errno << std::endl;
                        std::cout << "closing connection, fd = " << fd << std::endl;
                        close(fd);
                        break;
                    }
                    else if (errno == EAGAIN)
                    {
                        std::cout << "recv returned EAGAIN, fd = " << fd << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 延长睡眠时间，避免繁忙等待
                    } 
                }
            }
        });
    }
    dag::IOManager::GetThis()->addEvent(sock_list_fd, dag::IOManager::READ,test_accept);
}

void test_iomaager()
{
    int portno = 8080;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    //设置套接字
    sock_list_fd = socket(AF_INET,SOCK_STREAM,0);
    if (sock_list_fd < 0)
    {
        error("Error creating socket..\n");
    }

    int yes = 1;
    setsockopt(sock_list_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_list_fd, (struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
        error("Error binding socket..\n");

    if (listen(sock_list_fd, 1024) < 0)
    {
        error("Error listening..\n");
    }

    printf("epoll echo server listening for connection on port: %d\n",portno);
    fcntl(sock_list_fd, F_SETFL,O_NONBLOCK);
    dag::IOManager iom(2);
    iom.addEvent(sock_list_fd, dag::IOManager::READ,test_accept);
}

int main(int argc,char* argv[])
{
    dag::set_hook_enable(true);
    test_iomaager();
    return 0;
}
