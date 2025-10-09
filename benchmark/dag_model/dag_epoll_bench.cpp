#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define PORT 8000

int make_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 128);
    make_nonblock(listen_fd);

    int epfd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                int conn_fd = accept(listen_fd, NULL, NULL);
                make_nonblock(conn_fd);
                ev.events = EPOLLIN | EPOLLET;  // 边缘触发
                ev.data.fd = conn_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else {
                char buf[4096];
                while (1) {
                    int len = read(fd, buf, sizeof(buf));
                    if (len == 0) {
                        close(fd);
                        break;
                    } else if (len < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("read");
                        close(fd);
                        break;
                    } else {
                        write(fd, buf, len);
                    }
                }
            }
        }
    }
    return 0;
}
#endif

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

#define MAX_EVENTS 64
#define BUFFER_SIZE 10240
#define PORT 8888 // Use a different port to avoid conflict

// Sets a file descriptor to non-blocking mode
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
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Create, bind, and listen
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    set_non_blocking(listen_sock);

    // 2. Create epoll instance and register the listening socket
    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = listen_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event);

    printf("Raw epoll server listening on port %d\n", PORT);

    // 3. The Event Loop
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == listen_sock) {
                // Handle new connection
                conn_sock = accept(listen_sock, NULL, NULL);
                set_non_blocking(conn_sock);
                event.events = EPOLLIN | EPOLLET; // Edge-Triggered
                event.data.fd = conn_sock;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &event);
            } else {
                // Handle client data
                int client_fd = events[i].data.fd;
                ssize_t bytes_read;
                // Edge-triggered mode requires reading until EAGAIN
                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    write(client_fd, buffer, bytes_read); // Echo
                }

                if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN)) {
                    // Client disconnected or error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                }
            }
        }
    }

    close(listen_sock);
    close(epoll_fd);
    return 0;
}
