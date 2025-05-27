#include "User.h"
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/epoll.h>
void errif(bool condition,const char *errmsg)
{
    if(condition)
    {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

int setnonblocking(int fd)
{
    int old_options = fcntl(fd,F_GETFL);
    int new_options = old_options | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_options);
    return old_options;
}

void addFd(int epfd,int fd,int flags)
{
    epoll_event ev;
    ev.events = flags;
    ev.data.fd = fd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
}