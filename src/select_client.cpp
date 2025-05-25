#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/select.h>
#include "User.h"
#define MAX_LENTH 2048
int main()
{
    int ret = 0;
    // 确定服务端ip地址和端口号
    const char *ip = "127.0.0.1";
    const int port = 8080;
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
    //  创建缓冲区
    char buf[MAX_LENTH];
    //  创建读事件集合，并初始化
    fd_set read_fds;
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        FD_ZERO(&read_fds);
        FD_SET(connfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        ret = select(connfd + 1, &read_fds, NULL, NULL, NULL);
        if (ret < 0)
        {
            std::cout << "selection failure\n";
            break;
        }
        else
        {
            memset(buf,0,MAX_LENTH);
            if(FD_ISSET(STDIN_FILENO,&read_fds))
            {
                // 从客户端向服务端发送
                std::string input;
                std::getline(std::cin,input);
                strcpy(buf,input.c_str());
                send(connfd,buf,sizeof(buf),0);
                continue;
            }
            if(FD_ISSET(connfd,&read_fds))
            {
                ret = recv(connfd,buf,MAX_LENTH,0);
                if(ret>0)
                {
                    buf[ret]='\0';
                    printf("服务器来消息>> %s\n",buf);
                }
                else
                {
                    break;
                }
            }
        }
    }
    close(connfd);
    return 0;
}