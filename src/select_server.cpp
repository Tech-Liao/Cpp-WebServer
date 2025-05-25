#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "User.h"
#include <fcntl.h>
#include <sys/select.h>
#define BUFFER_SIZE 2048 // 读写缓存大小
#define USER_LIMIT 5
int main()
{
    //  监听返回值
    int ret{0};
    // 假设IPv4地址
    const char *ip = "127.0.0.1";
    const int port = 8080;
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
    char buf[BUFFER_SIZE];               // 缓存区
    int fd[USER_LIMIT];                  //  客户端对应的文件描述符
    int user_count = 0;                  //  已建立客户端数量
    for (int i = 0; i < USER_LIMIT; i++) // 初始化文件描述符数组
        fd[i] = -1;
    // 创建读事件集合
    fd_set readfds;
    int max_fd, curfd, addrlen;
    struct sockaddr_in cl_addr;
    max_fd = sockfd;
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        for (int i = 0; i < user_count; i++)
        {
            curfd = fd[i];
            if (curfd > 0)
                FD_SET(curfd, &readfds);
        }
        ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        errif(ret < 0, "select failure");
        if (FD_ISSET(sockfd, &readfds))
        {
            // 有客户端连接
            addrlen = sizeof(cl_addr);
            int new_sockfd = accept(sockfd, (struct sockaddr *)&cl_addr, (socklen_t *)&addrlen);
            if (new_sockfd < 0)
            {
                std::cout << "errno is:" << errno << std::endl;
                continue;
            }
            if (user_count >= USER_LIMIT)
            {
                const char *info = "too many users\n";
                std::cout << info;
                send(new_sockfd, info, strlen(info), 0);
                close(new_sockfd);
                continue;
            }
            for (int i = 0; i < USER_LIMIT; i++)
            {
                if (-1 == fd[i])
                {
                    fd[i] = new_sockfd;
                    user_count++;
                    max_fd = ((max_fd > new_sockfd) ? max_fd : new_sockfd);
                    break;
                }
            }
        }
        for (int i = 0; i < USER_LIMIT; i++)
        {
            curfd = fd[i];
            if (curfd != -1 && FD_ISSET(curfd, &readfds))
            {
                memset(buf, '\0', BUFFER_SIZE);
                ret = recv(curfd, buf, BUFFER_SIZE-1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, buf, curfd);
                if(ret >0)
                {
                    buf[ret]=0;
                    printf("%d >> %s\n",curfd,buf);
                    for(int j=0;j<USER_LIMIT;j++)
                    {
                        if(-1 != fd[j] && fd[j]!=curfd)
                            send(fd[j],buf,strlen(buf),0);
                    }
                }
                else
                {
                    printf("客户端:%d 已断开连接\n",curfd);
                    fd[i] = -1;
                    user_count--;
                    close(curfd);
                }
            }
        }
    }
    close(sockfd);
    return 0;
}