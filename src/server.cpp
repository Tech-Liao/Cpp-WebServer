#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "User.h"
#include <fcntl.h>
#define MAX_LENTH 1024

int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
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
    Dealerrno(ret == -1, "inet_aton失败");
    // 创建通信文件描述符，并不知道通信的地址
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    Dealerrno(sockfd == -1, "socket创建失败");
    // 绑定通信地址
    ret = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    Dealerrno(ret == -1, "bind创建失败");
    // 监听通信文件描述符，是否有客户端连接
    ret = listen(sockfd, 5);
    Dealerrno(ret == -1, "listen创建失败");
    // sleep(10);
    // 发现监听队列有客户连接，需要接受连接

    while (true)
    {
        // 客户端地址
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t clt_addr_len = sizeof(client_addr);
        int connfd = accept(sockfd, (sockaddr *)&client_addr, &clt_addr_len);
        Dealerrno(connfd == -1, "accept失败");
        // 进程间通信
        int pipefd[2];
        ret = pipe(pipefd);
        setnonblocking(pipefd[0]);
        setnonblocking(pipefd[1]);
        Dealerrno(ret == -1,"pipe创建失败");
        if (fork() == 0)
        {
            // 在子进程中
            //  1.关闭监听sockfd,pipefd[0]
            close(sockfd);
            close(pipefd[0]);
            //  2.执行功能
            char buf[MAX_LENTH];
            while (true)
            {
                memset(buf, 0, sizeof(buf));
                ret = recv(connfd, buf, sizeof(buf), MSG_WAITALL);
                if (ret == 0)
                {   
                    //这里表示客户端断开连接
                    char *str="client already closed!";
                    std::cout << str<<std::endl;
                    write(pipefd[1],str,sizeof(str));
                    close(connfd);
                    break;
                }
                ret = send(connfd, buf, sizeof(buf), MSG_WAITALL);
                Dealerrno(ret == -1, "send失败");
            }
            //  3. 退出子进程
            exit(-1);
        }
        //父进程关闭connfd连接
        close(connfd);
        close(pipefd[1]);
        char buf[MAX_LENTH];
        memset(buf,0,sizeof(buf));
        read(pipefd[0],buf,sizeof(buf));
        std::cout<<buf;
    }
    close(sockfd);
    return 0;
}