#include "SocketInfo.h"
#include <netdb.h>
#include <netinet/tcp.h>
#include "headerRequest.h"


int socket_connect(const char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     

	if((hp = gethostbyname(host)) == NULL){
		perror("gethostbyname");
		exit(1);
	}

    memcpy(&addr.sin_addr,hp->h_addr_list[0], hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);
	}
	return sock;
}


SocketInfo::SocketInfo(ServerSocketInfo *server) {
    sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    this->fd  = accept(server->getListenFD(), (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
    if (this->fd == -1) {
        perror("Accept socket error");
    }
    //std::cout<<"New Client Request : "<<this->fd<<std::endl;
}

int SocketInfo::getFD() {
    return this->fd;
}

void * SocketInfo::read(void *params) {
    SocketInfo *socket = (SocketInfo *)params;
    int bytes_read = recv(socket->fd, socket->buff, sizeof(socket->buff), 0);
      if(bytes_read>0) {
         std::string str(socket->buff, bytes_read);
         Request req = Request::deserialize(str);
         char *host =  (char *)req.getHeaderValue("Host").c_str(); //"devimages.apple.com";
         int conn_fd = socket_connect(host, 80);
         std::cout<<"Connected to host: "<<host<<std::endl;
        write(conn_fd, socket->buff, bytes_read);
        std::cout<<"Written to host: "<<host<<std::endl;
        int read_bytes = INT_MAX;
        char buffer[1024] = {0};
        do {
            read_bytes = recv(conn_fd, buffer, 1024, 0);
            write(socket->fd, buffer,read_bytes);
            bzero(buffer, 1024);
        } while(read_bytes > 0);
        
        shutdown(conn_fd, SHUT_RDWR); 
	    close(conn_fd); 
     }
    //printf("Executed the task\n");
}


SocketInfo::~SocketInfo() {
      close(fd);
      std::cout<<"Client has disconnected client FD: ["<<fd<<"]\n";
}


//--------------------------------------------------------
// -----------Server Part
ServerSocketInfo::ServerSocketInfo(char *ip, int port) {
    socket_listen_fd = 0;
    sockaddr_in serv_addr;
    this->port = port;

    if (((socket_listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    int optval = 1;
    setsockopt(socket_listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (bind(socket_listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }
    // Start listening.
    if (listen(socket_listen_fd, 20)< 0) {
        perror("Error listen socket");
        exit(1);
    }
}

int ServerSocketInfo::getListenFD() {
    return socket_listen_fd;
}

int ServerSocketInfo::write(char *buff, int len) {
    return 1;
}
