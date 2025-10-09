#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAX_EVENTS 10   // epoll_wait 返回的最大事件数
#define BUFFER_SIZE 512 // 缓冲区大小
#define PORT 8888       // 监听端口

// 函数：设置文件描述符为非阻塞模式
void set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int listen_sock, conn_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. 创建监听 socket
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置 SO_REUSEADDR 选项，允许端口快速重用
    int opt = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // 2. 绑定地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网络接口
    server_addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // 3. 开始监听
    if (listen(listen_sock, 5) == -1) {
        perror("listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // 将监听 socket 设置为非阻塞
    set_non_blocking(listen_sock);

    // 4. 创建 epoll 实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    struct epoll_event event, events[MAX_EVENTS];

    // 5. 将监听 socket 添加到 epoll 中
    event.events = EPOLLIN; // 监听读事件（新连接）
    event.data.fd = listen_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event) == -1) {
        perror("epoll_ctl: listen_sock");
        close(listen_sock);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Echo server is listening on port %d\n", PORT);

    // 6. 事件循环
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // -1 表示无限等待
        if (num_events == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == listen_sock) {
                // a. 处理新的客户端连接
                conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
                if (conn_sock == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // 由于是 listen_sock 是非阻塞的，可能没有连接可接受
                        printf("accept returned EAGAIN or EWOULDBLOCK\n");
                    } else {
                        perror("accept");
                    }
                    continue;
                }

                // 将新的客户端 socket 设置为非阻塞
                set_non_blocking(conn_sock);

                // 将新的客户端 socket 添加到 epoll
                event.events = EPOLLIN | EPOLLET; // 监听读事件，并使用边缘触发(ET)模式
                event.data.fd = conn_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &event) == -1) {
                    perror("epoll_ctl: conn_sock");
                    close(conn_sock);
                } else {
                     printf("New connection from %s:%d (fd: %d)\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), conn_sock);
                }

            } else {
                // b. 处理已连接客户端的数据
                int client_fd = events[i].data.fd;
                ssize_t bytes_read;

                // 由于使用了边缘触发(ET)，必须循环读取，直到缓冲区为空(EAGAIN)
                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    // echo 数据回客户端
                    write(client_fd, buffer, bytes_read);
                }

                if (bytes_read == 0) {
                    // 客户端关闭了连接
                    printf("Client (fd: %d) disconnected.\n", client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); // 从 epoll 中移除
                    close(client_fd);
                } else if (bytes_read == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        // 发生了一个真正的错误
                        perror("read error");
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                        close(client_fd);
                    }
                }
            }
        }
    }

    // 7. 清理资源
    close(listen_sock);
    close(epoll_fd);

    return 0;
}
