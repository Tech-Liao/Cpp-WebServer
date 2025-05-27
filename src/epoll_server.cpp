#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <vector>
#include "User.h"

#define BUFFER_SIZE 2048 // 读写缓存大小
#define EVENT_LIMIT 1024 // 事件表大小
int main()
{
    //  监听返回值
    int ret{0};
    // 假设IPv4地址
    const char *ip = "127.0.0.1";
    const int port = 9999;
    // 创建IPV4专用地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;           // 代表地址族类型，AF_INET代表IPV4地址族
    addr.sin_port = port;                // 端口号
    ret = inet_aton(ip, &addr.sin_addr); // 将IP地址转换成网络通信中所需要的类型
    errif(ret == -1, "inet_aton失败");
    // 创建通信文件描述符，并不知道通信的地址
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket创建失败");
    // 绑定通信地址
    ret = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    errif(ret == -1, "bind创建失败");
    // 监听通信文件描述符，是否有客户端连接
    ret = listen(sockfd, 5);
    errif(ret == -1, "listen创建失败");

    int epfd = epoll_create(1);
    std::vector<int> client;
    addFd(epfd, sockfd, EPOLLIN);
    char buf[BUFFER_SIZE];
    while (1)
    {
        epoll_event evs[EVENT_LIMIT];
        ret = epoll_wait(epfd, evs, EVENT_LIMIT, -1);
        errif(ret < 0, "epoll wait failure");
        for (int i = 0; i < ret; i++)
        {
            int curfd = evs[i].data.fd;
            if (curfd == sockfd)
            {
                sockaddr_in cl_addr;
                socklen_t len = sizeof(cl_addr);
                int new_connfd = accept(curfd, (struct sockaddr *)&cl_addr, &len);
                errif(new_connfd < 0, "accept failure");

                client.push_back(new_connfd);
                addFd(epfd, new_connfd, EPOLLIN);
                setnonblocking(new_connfd);
                std::string welcome_msg = "Welcome to the char room\n";
                send(new_connfd, welcome_msg.c_str(), welcome_msg.size(), 0);
                std::cout << "New client:" << new_connfd << "\n";
            }
            else if (evs[i].events & EPOLLIN)
            {
                memset(buf, '\0', BUFFER_SIZE);
                ret = recv(curfd, buf, BUFFER_SIZE - 1, 0);
                if (ret < 0)
                {
                    std::cout << curfd << " left\n";
                    close(curfd);
                    break;
                }
                else
                {
                    for (int i = 0; i < client.size(); i++)
                    {
                        if (client[i] != curfd)
                            ret = send(client[i], buf, strlen(buf), 0);
                    }
                }
            }
        }
    }
    close(sockfd);
    close(epfd);
    return 0;
}