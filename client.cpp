#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<iostream>
int main()
{
    //确定服务端ip地址和端口号
    const char * ip  = "127.0.0.1";
    const int port = 8080;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(ip,&server_addr.sin_addr);
    server_addr.sin_port = port;

    int connfd = socket(AF_INET,SOCK_STREAM,0);
    int ret = connect(connfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(ret == 0)
    {
        std::cout<<"connection success\n";
    }
    else
    {
        std::cout<<"connection failure\n";
    }
    return 0;
}