#ifndef __SOCKETINFO_H__
#define __SOCKETINFO_H__

#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>

class ServerSocketInfo {
private:
    int socket_listen_fd;
    int port;

public:
    ServerSocketInfo(char *ip, int port, char *hostEndPoint, short hostEndPointPort);
    int getListenFD();
    int write(char *buff, int len);

    char *hostEndPoint;
    short hostEndPointPort;
};

class SocketInfo {
public:  
  ServerSocketInfo *serverCtx;
  int fd;
  char buff[1024];
  char parsedLen;
  int TotalLen;
  SocketInfo(ServerSocketInfo *server);
  int getFD();
  static void *read(void *params);
  ~SocketInfo();
};


#endif // __SOCKETINFO_H__