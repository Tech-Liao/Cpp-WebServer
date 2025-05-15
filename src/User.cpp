#include "User.h"
void Dealerrno(bool flag, std::string msg)
{
    if (flag)
    {
        std::cout << msg << "\n";
        exit(-1);
    }
}