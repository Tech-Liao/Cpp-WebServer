#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include<sys/types.h>
#include<unistd.h>
#include "User.h"
#define MAX_LENTH 1024
int main()
{
    int ret;
    // 假设IPv4地址
    const char *ip = "127.0.0.1";
    const int port = 8080;
    // 创建IPV4专用地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;     // 代表地址族类型，AF_INET代表IPV4地址族
    addr.sin_port = port;          // 端口号
    ret = inet_aton(ip, &addr.sin_addr); // 将IP地址转换成网络通信中所需要的类型
    Dealerrno(ret==-1,"inet_aton失败");
    // 创建通信文件描述符，并不知道通信的地址
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    Dealerrno(sockfd==-1,"socket创建失败");
    // 绑定通信地址
    ret = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    Dealerrno(ret==-1,"bind创建失败");
    // 监听通信文件描述符，是否有客户端连接
    ret = listen(sockfd, 5);
    Dealerrno(ret==-1,"listen创建失败");
    // sleep(10);
    // 发现监听队列有客户连接，需要接受连接
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t clt_addr_len = sizeof(client_addr);
    // 连接成功重新创建一个sockfd，通过这个sockfd进行通信发送，不成功，则返回-1
    int connfd = accept(sockfd, (sockaddr *)&client_addr, &clt_addr_len);
    Dealerrno(connfd==-1,"accept失败");
    if (connfd < 0)
    {
        std::cout << "errno is " << errno << std::endl;
    }
    else
    {
        //成功打印客户端ip和端口号
        char remote[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&client_addr.sin_addr,remote,INET_ADDRSTRLEN);
        std::cout<<"connected with ip:"<<remote<<" and port:"<<ntohs(client_addr.sin_port)<<std::endl;
        char buf[MAX_LENTH];
        while(true)
        {
            memset(buf,0,sizeof(buf));
            ret = recv(connfd,buf,sizeof(buf),MSG_WAITALL);
            if(ret == 0)
            {
                std::cout<<"client already closed!\n";
                close(connfd);
            }
            ret =send(connfd,buf,sizeof(buf),MSG_WAITALL);
            Dealerrno(ret==-1,"send失败");
        }
    }
    close(sockfd);
    return 0;

}