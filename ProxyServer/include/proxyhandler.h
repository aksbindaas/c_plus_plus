#ifndef __PROXYHANDLER_H__
#define __PROXYHANDLER_H__

#include "threadPool.h"
#include "SocketInfo.h"
#include <sys/event.h>
#include <pthread.h>
#include <map>
class ProxyHandler {

public:
    ThreadPool *pool;
    ServerSocketInfo *socketInfo;
    pthread_mutex_t mtxList = PTHREAD_MUTEX_INITIALIZER;
    std::map<int, SocketInfo *> fdList;
    int kq;
    pthread_t proxyHandle;
    pthread_t rawDataHandle;
    struct kevent change_event[4];
public:
    ProxyHandler(char *ip, short port);
    ~ProxyHandler ();
    static void *rawPacket(void *args);
    static void *connectionHandler(void *args);

};
#endif // __PROXYHANDLER_H__