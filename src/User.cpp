#include "User.h"
#include<iostream>

void errif(bool flag,const char *msg)
{
    if(flag)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}