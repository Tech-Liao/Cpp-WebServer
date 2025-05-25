#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "User.h"
#include <fcntl.h>
#include <poll.h>
#define BUFFER_SIZE 2048 // 读写缓存大小
#define USER_LIMIT 5
#define FD_LIMIT 65535

struct client_data
{
    sockaddr_in address;
    char *write_buf;
    char buf[BUFFER_SIZE];
};

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

    client_data *users = new client_data[FD_LIMIT]; // 创建users数组
    pollfd fds[USER_LIMIT + 1];                     //  客户端连接数组
    int user_count = 0;                             // 已连接客户端数量
    for (int i = 1; i <= USER_LIMIT; i++)           // 初始化
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = sockfd; // 监听sockfd是否有连接
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while (1)
    {
        ret = poll(fds, USER_LIMIT + 1, -1);
        errif(ret < 0, "poll failure");
        for (int i = 0; i < user_count + 1; i++)
        {
            if ((fds[i].fd == sockfd) && (fds[i].revents & POLLIN))
            {
                struct sockaddr_in cl_addr;
                socklen_t addrlen = sizeof(cl_addr);
                int new_connfd = accept(sockfd, (struct sockaddr *)&cl_addr, &addrlen);
                if (new_connfd < 0)
                {
                    std::cout << "errno is " << errno << std::endl;
                    continue;
                }
                if (user_count >= USER_LIMIT)
                {
                    const char *info = "too many users\n";
                    std::cout << info;
                    send(new_connfd, info, strlen(info), 0);
                    close(new_connfd);
                    continue;
                }
                user_count++;
                users[new_connfd].address = cl_addr;
                users[new_connfd].write_buf = NULL;
                setnonblocking(new_connfd);
                fds[user_count].fd = new_connfd;
                fds[user_count].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_count].revents = 0;
                std::cout << "comes a new user,now have " << user_count << " users\n";
            }
            else if (fds[i].revents & POLLERR)
            {
                std::cout << "get an error from " << fds[i].fd << "\n";
                char errors[100];
                memset(errors, 0, sizeof(errors));
                socklen_t len = sizeof(errors);
                if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &len))
                    std::cout << "get socket option failed\n";
                continue;
            }
            else if (fds[i].revents & POLLRDHUP)
            {
                users[fds[i].fd] = users[fds[user_count].fd];
                close(fds[i].fd);
                fds[i] = fds[user_count];
                i--;
                user_count--;
                std::cout << "a client left\n";
            }
            else if (fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                std::cout << "get " << ret << " bytes of client data " << users[connfd].buf << " from " << connfd << std::endl;
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_count].fd];
                        fds[i] = fds[user_count];
                        i--;
                        user_count--;
                    }
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int j = 1; j <= user_count; j++)
                    {
                        if (fds[j].fd == connfd)
                            continue;
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }
            }
            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if (!users[connfd].write_buf)
                    continue;
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete[] users;
    close(sockfd);
    return 0;
}