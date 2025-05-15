#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<iostream>
#include "User.h"
#define MAX_LENTH 1024
int main()
{
    int ret=0;
    //确定服务端ip地址和端口号
    const char * ip  = "127.0.0.1";
    const int port = 8080;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    ret = inet_aton(ip,&server_addr.sin_addr); 
    Dealerrno(ret == 0,"inet_aton失败");
    server_addr.sin_port = port;

    int connfd = socket(AF_INET,SOCK_STREAM,0);
    Dealerrno(connfd==-1,"socket创建失败");
    ret = connect(connfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    Dealerrno(ret==-1,"connect连接失败");
    if(ret == 0)
    {
        std::cout<<"connection success\n";
        char buf[MAX_LENTH];
        while(true)
        {
            memset(buf,0,sizeof(buf));
            std::cin>>buf;
            if(!strcmp(buf,"QUIT"))
            {
                close(connfd);
                break;
            }
            ret = send(connfd,buf,sizeof(buf),MSG_WAITALL);
            Dealerrno(ret == -1,"send失败");
            memset(buf,0,sizeof(buf));
            ret = recv(connfd,buf,sizeof(buf),MSG_WAITALL);
            Dealerrno(ret == -1,"recv失败");
            std::cout<<buf<<std::endl;
        }
    }
    else
    {
        std::cout<<"connection failure\n";
    }
    return 0;
}