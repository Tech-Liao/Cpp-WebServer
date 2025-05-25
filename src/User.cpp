#include "User.h"
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
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