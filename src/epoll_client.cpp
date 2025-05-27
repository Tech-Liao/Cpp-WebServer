#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include "User.h"
#define BUFFER_SIZE 2048
#define EVENT_LIMIT 1024
int main()
{
    int ret = 0;
    // 确定服务端ip地址和端口号
    const char *ip = "127.0.0.1";
    const int port = 9999;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    ret = inet_aton(ip, &server_addr.sin_addr);
    server_addr.sin_port = port;

    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(connfd == -1, "socket create failure");
    ret = connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    errif(ret == -1, "connect failure");
    std::cout << "connection success\n";

    // 使用epoll来实现聊天室功能
    int epfd = epoll_create(1); // 创建epoll文件描述符，传入参数是提示内核事件表需要多大
    addFd(epfd, STDIN_FILENO, EPOLLIN);
    addFd(epfd, connfd, EPOLLIN);
    char buf[BUFFER_SIZE];
    setnonblocking(connfd);
    while (true)
    {
        epoll_event evs[EVENT_LIMIT];
        ret = epoll_wait(epfd, evs, EVENT_LIMIT, -1);
        if (ret < 0)
        {
            std::cerr << "epoll wait failure\n";
            break;
        }
        for (int i = 0; i < ret; i++)
        {
            int curfd = evs[i].data.fd;
            if ((evs[i].events & EPOLLIN) &&curfd == STDIN_FILENO)
            {
                memset(buf, 0, BUFFER_SIZE);
                ret = read(STDIN_FILENO, buf, BUFFER_SIZE - 1);
                buf[ret] = '\0';
                if (ret > 0)
                    send(connfd, buf, strlen(buf), 0);
                else if (ret == 0)
                {
                    continue;
                }
            }
            else if (evs[i].events & EPOLLIN)
            {
                memset(buf, 0, BUFFER_SIZE);
                ret = recv(connfd, buf, BUFFER_SIZE - 1, 0);
                if (ret > 0)
                {
                    buf[ret] = '\0';
                    std::cout << "Received: " << buf;
                }
                else if (ret == 0)
                {
                    std::cout << "Server disconnected\n";
                    close(connfd);
                    close(epfd);
                    return 0;
                }
                else
                {
                    std::cerr << "recv failure\n";
                    close(connfd);
                    close(epfd);
                    return -1;
                }
            }
        }
    }

    close(connfd);
    close(epfd);
    return 0;
}