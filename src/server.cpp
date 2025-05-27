#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "User.h"
#include <fcntl.h>
#include<sys/epoll.h>
#include<vector>
#include<iterator>
#include <algorithm>
#define BUFFER_SIZE 1024
#define USER_LIMIT 5
#define EVENT_LIMIT 1024
int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addFd(int epfd,int fd,int flag)
{
    epoll_event ev;
    ev.data.fd=fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
}

int main()
{
    int ret;
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
    char buf[BUFFER_SIZE];
    int epfd = epoll_create(1);         //  创建epoll
    addFd(epfd,sockfd,EPOLLIN);
    std::vector<int> users;
    while (true)
    {
        epoll_event evs[EVENT_LIMIT];
        ret = epoll_wait(epfd,evs,EVENT_LIMIT,-1);
        errif(ret<0,"epoll wait failure");
        for(int i=0,nums=ret;i<nums;i++)
        {
            int curfd = evs[i].data.fd;
            if(curfd == sockfd)
            {
                sockaddr_in cl_addr;
                socklen_t len = sizeof(cl_addr);
                int connfd = accept(curfd,(struct sockaddr *)&cl_addr,&len);
                errif(connfd==-1,"accept failure");
                if(users.size()>=USER_LIMIT)
                {
                    const char *msg = "server had too many user\n";
                    send(connfd,msg,strlen(msg),0);
                    close(connfd);
                    continue;
                }
                users.push_back(connfd);
                addFd(epfd,connfd,EPOLLIN);
                setnonblocking(connfd);
            }
            else if(evs[i].events & EPOLLIN)
            {
                memset(buf,0,BUFFER_SIZE);
                ret =recv(curfd,buf,BUFFER_SIZE-1,0);
                if(ret<0)
                {
                    std::cout<<curfd<<"had left\n";
                    users.erase(std::remove(users.begin(), users.end(), curfd), users.end());
                    std::cout<<users.size()<<std::endl;
                    close(curfd);
                    continue;
                }
                else
                {
                    buf[ret]='\0';
                    send(curfd,buf,strlen(buf),0);
                }
            }
        }
    }
    close(epfd);
    close(sockfd);
    return 0;
}