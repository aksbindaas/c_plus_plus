#include "proxyhandler.h"
#include <iostream>

ProxyHandler::ProxyHandler(char *ip, short port, char *hostEndPoint, short hostEndPointPort) {
    pool = new ThreadPool(50);
    socketInfo = new ServerSocketInfo(ip, port, hostEndPoint, hostEndPointPort);
    kq = kqueue();
    EV_SET(change_event, socketInfo->getListenFD(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    if (kevent(kq, change_event, 1, NULL, 0, NULL) == -1) {
        perror("kevent");
        exit(1);
    }
    
    if (pthread_create(&proxyHandle, nullptr, &ProxyHandler::connectionHandler, this)) {
        perror("ProxyHandler thread");
        exit(1);
    }

    // if (pthread_create(&rawDataHandle, nullptr, &ProxyHandler::rawPacket, this)) {
    //     perror("ProxyHandler thread");
    //     exit(1);
    // }
}

ProxyHandler::~ProxyHandler() {

    pthread_join(proxyHandle, nullptr);
    //pthread_join(rawDataHandle, nullptr);
    delete pool;
    delete socketInfo;
}

// void *ProxyHandler::rawPacket(void *args) {
//     ProxyHandler *handler = static_cast<ProxyHandler*>(args);
//     handler->pool->doWork(&Sniffer::sniffer, nullptr);
// }

void *ProxyHandler::connectionHandler(void *args) {
    ProxyHandler *handler = static_cast<ProxyHandler*>(args);
    int kq = handler->kq;
    struct kevent event[4];
    int socket_connection_fd;
    
    while (1) {
        int new_events = kevent(kq, NULL, 0, event, 1, NULL);
        if (new_events == -1)
        {
            perror("kevent");
            exit(1);
        }

        for (int i = 0; new_events > i; i++)
        {
            int event_fd = event[i].ident;

            // When the client disconnects an EOF is sent. By closing the file descriptor the event is automatically removed from the kqueue.
            if (event[i].flags & EV_EOF) {
                pthread_mutex_lock(&handler->mtxList);
                SocketInfo *info =  handler->fdList[event_fd];
                handler->fdList.erase(event_fd);
                delete info;
                pthread_mutex_unlock(&handler->mtxList);
                
            }
            // If the new event's file descriptor is the same as the listening socket's file descriptor, we are sure that a new client wants 
            // to connect to our socket.
            else if (event_fd == handler->socketInfo->getListenFD())
            {
                SocketInfo *sock = new SocketInfo(handler->socketInfo);
                pthread_mutex_lock(&handler->mtxList);
                handler->fdList[sock->getFD()] = sock;
                pthread_mutex_unlock(&handler->mtxList);

                // Put this new socket connection also as a 'filter' event to watch in kqueue, so we can now watch for events on this new socket.
                EV_SET(handler->change_event, sock->getFD(), EVFILT_READ , EV_ADD, 0, 0, NULL);
                if (kevent(kq, handler->change_event, 1, NULL, 0, NULL) < 0)
                {
                    perror("kevent error");
                }
            }

            else if (event[i].filter & EVFILT_READ) {
                // Read bytes from socket
                pthread_mutex_lock(&handler->mtxList);
                SocketInfo *info =  handler->fdList[event_fd];
                pthread_mutex_unlock(&handler->mtxList);
               // std::cout<<"Prepare task for fd "<<event_fd<<std::endl;
                handler->pool->doWork(&SocketInfo::read, info);
            }
        }
    }
}