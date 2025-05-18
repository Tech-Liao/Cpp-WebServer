# Cpp-WebServer
学习cpp服务器编程
### commit faa1c6c

这次提交是实现了最简单的客户端与服务端通信连接。

在Linux服务端中，总体逻辑：

1. 确定服务端IP地址与端口号
2. 创建socket文件描述符
3. 将socket文件描述符与ip地址、端口号进行绑定
4. socket文件描述符放入监听队列，查看socket文件描述符是否有客户端连接
5. 接受客户端的连接，返回新的socket文件描述符，通过该文件描述符进行通信。

在客户端中，总体逻辑：

1. 确定服务端IP地址和端口号
2. 创建socket文件描述符
3. 发起连接请求，通过已经创建的socket进行通信

#### 程序所需要的API

地址族

![image-20250515163057725](./assets/image-20250515163057725.png)

在通信过程中，不单单需要ip地址，还需要其他信息，因此设计了存储IP地址的结构体。

存储IP地址的专用地址结构体

```c++
#include<sys/socket.h>
//IPV4专用地址
struct socket_in
{
    sa_family_t sin_family;	//表示地址族类型 AF_INET或AF_INET6
    u_int16_t port;
    struct in_addr sin_addr;
};
struct in_addr
{
    u_int32_t s_addr;
};
//通用地址
struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[14];
};
```

所有的专用地址在使用socket的API都需要强行转换成通用地址，可以直接强行转换。

在网络通信时，IP地址并不是常用的点分十进制形式，而且特殊的二进制形式，因此设计了将点分十进制转换成二进制数据

```c++
#include<arpa/inet.h>

in_addr_t inet_addr(const char *src); //将点分十进制转换成二进制数，并返回二进制数
int inet_aton(const char *src,struct in_addr *dest); //将IPv4地址转换到dest。
```

socket（套接字）是Linux/Unix设计进程间或应用间通信机制或接口。

创建socket文件描述符

```c++
#include<sys/types.h>
#include<sys/socket.h>

int socket(int domain,int type,int protoocl);
// domain:协议族，type：socket通信服务类型，protocol：由前两个参数共同决定，常用0（暂不了解）
/*
	type：sock_stream=tcp
		sock_ugram=udp
	成功返回socket文件描述符
	失败返回-1，并设置errno
*/
```

绑定socket地址，因为没有地址无法通行。

```c++
#include<sys/types.h>
#include<sys/socket.h>

int bind(int sockfd，struct sockaddr *addr,socklen_t addrlen);
/*
	成功返回0，失败返回-1并设置errno
*/
```

监听socket，是将待连接的客户端进行处理

```c++
#include<sys/socket.h>
int listen(int sockfd,int backlog);
// backlog:表示监听队列最大长度，成功返回0，失败返回-1并设置errn
```

发起连接

```c++
#include<sys/socket.h>
int conntion(int sockfd,struct sockaddr *serv_addr,socklen_t servaddr_len);
// 成功返回0，失败返回-1并设置errno
```

接受连接

```c++
#include<sys/socket.h>
int accept(int sockfd,struct sockaddr *client_addr,socklen_t *client_addr_len);
//成功返回新的sockfd进行通信，失败返回-1并设置errno
```

### commit 180141d

本次提交主要用Cmake重构项目，使项目编译过简化，便于维护，其次实现echo功能，最后用一个函数来接受返回处理错误。

echo服务：客户端发送信息给服务端，服务端将信息回传给客户端。

![image-20250518151917215](./assets/image-20250518151917215.png)

#### 涉及API

```c++
// socket创建的文件描述符可以使用文件的read/write读写
// 这次主要介绍socket
ssize_t recv(int sockfd,void*buf,size_t len,int flags);	
/*
	sockfd写入len个字符，到buf。
	flag：读取控制，MSG_DONTWAIT--该操作非阻塞
*/
ssize_t send(int sockfd,const void*buf,size_t len,int flags);
/*
	buf中len字符写入sockfd中
*/

```

#### 修改部分

```c++
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

#include "User.h"
void Dealerrno(bool flag, std::string msg)
{
    if (flag)
    {
        std::cout << msg << "\n";
        exit(-1);
    }
}
```

