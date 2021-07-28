#include <iostream>
#include "proxyhandler.h"

int main(int argc, char *argv[])
{
    // argv[1] == PORT
    // argv[2] == HOST
    // argv[3] == PORT
   ProxyHandler proxy("192.168.0.187", atoi(argv[1]), argv[2], atoi(argv[3]));
   getchar();
    return 0;
}