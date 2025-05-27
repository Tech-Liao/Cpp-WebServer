#ifndef USER_H
#define USER_H
void errif(bool condition,const char *errmsg);
int setnonblocking(int fd);
void addFd(int epfd,int fd,int flags);
#endif