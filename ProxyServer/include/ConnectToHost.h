#ifndef __CONNECTTOHOST_H__
#define __CONNECTTOHOST_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

class RemoteConnection {
private:
    int conn_fd;
public: 
    int Remote_connect(const char *host, in_port_t port) {
        struct hostent *hp;
        struct sockaddr_in addr;
           

        if((hp = gethostbyname(host)) == NULL){
            perror("gethostbyname");
            exit(1);
        }

        memcpy(&addr.sin_addr,hp->h_addr_list[0], hp->h_length);
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        conn_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        int on = 1;
        setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

        if(conn_fd == -1){
            perror("setsockopt");
            exit(1);
        }
        
        if(connect(conn_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
            perror("connect");
            exit(1);
        }
        
        sockaddr_in client;
        socklen_t clientsz = sizeof(client);
        getsockname(conn_fd, (struct sockaddr *) &client, &clientsz);
        return (client.sin_port);
        //return sock;
    }

    void Remote_Write(unsigned char buff[], int len) {
        if(write(conn_fd, buff, len) <= 0) {
              //printf("*******************************************Enable to write");
        }
    }

    ~RemoteConnection() {
       shutdown(conn_fd, SHUT_RDWR); 
       close(conn_fd); 
    }



};
#endif // __CONNECTTOHOST_H__