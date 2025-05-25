#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include "User.h"
#define BUFFER_SIZE 2048
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
    //创建pollfd数组，将连接sockfd与标准输入放入监听
    pollfd fd[2];
    fd[0].fd = connfd;
    fd[0].events = POLLIN | POLLRDHUP;
    fd[0].revents=0;
    fd[1].fd=STDIN_FILENO;
    fd[1].events = POLLIN;
    fd[1].revents=0;
    char buf[BUFFER_SIZE];
    setnonblocking(connfd);

    int pipefd[2];
    ret = pipe(pipefd);
    errif(ret<0,"pipefd创建失败");

    while (true)
    {
        ret = poll(fd,2,-1);
        if(ret<0)
        {
            perror("poll失败");
            break;
        }
        if(fd[1].revents & POLLIN)
        {
            memset(buf,'\0',BUFFER_SIZE);
            std::string input;
            getline(std::cin,input);
            strcpy(buf,input.c_str());
            send(connfd,buf,strlen(buf),0);
        }
        if(fd[0].revents & POLLRDHUP)
        {
            std::cout<<"server close the connection\n";
            break;
        }
        else if(fd[0].revents & POLLIN)
        {
            memset(buf,'\0',BUFFER_SIZE);
            ret = recv(fd[0].fd,buf,BUFFER_SIZE,0);
            std::cout<<buf<<std::endl;
        }
    }
    close(connfd);
    return 0;
}