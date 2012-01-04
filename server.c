#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]){
	int error, socketfd, cfd;
	struct addrinfo hints;
	struct addrinfo *result, *res;
	struct sockaddr peer_sockaddr;
	socklen_t peer_sockaddr_size;
	char port[] = "10012";
	int MAX_CONN = 128;
	int MAX_BUFF = 1024;
	char buff[MAX_BUFF];
    int i, n;
    
    fd_set fds, fds_backup;
    int fdmax;

	printf("Port: %s\n", port);
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags |= AI_PASSIVE;
	hints.ai_protocol = 0;

	if ((error = getaddrinfo(NULL, port, &hints, &result))){
		fprintf(stderr, "getaddrinfo(%p, %s, %p, %p)  got error %s \n", 
                NULL, port, &hints, &result, gai_strerror(error));
        perror("getaddrinfo() error ");
		exit(-1);
	}

    /* Loop all to find available socket to bind socket*/
	for (res = result; res != NULL; res = res->ai_next) {
		socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketfd == -1)
			continue;
		if (bind(socketfd, res->ai_addr, res->ai_addrlen) != -1) {
			fprintf(stderr, "bind() OK !\n");
			break;
		}
		close(socketfd);
	}
    
    if (res == NULL) {
        perror("bind() error ");
        exit(-1);
    }
    
    freeaddrinfo(result);

	/* listen to incoming connection */
	if (listen(socketfd, MAX_CONN) == -1) {
	    perror("listen() error ");
        exit(-1);
	}
    
    FD_ZERO(&fds_backup);
    FD_SET(socketfd, &fds_backup);
    /* Set max file descriptor up till now */
    fdmax = socketfd;
    
    for (;;) {
        fds = fds_backup;
        if (select(fdmax+1, &fds, NULL, NULL, NULL) < 0) {
            perror("select() error ");
            exit(1);
        }
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &fds)) {
                if(i == socketfd) {
                    /* accept connections */
                    peer_sockaddr_size = sizeof(peer_sockaddr);
                    cfd = accept(socketfd, &peer_sockaddr, &peer_sockaddr_size);
                    if (cfd == -1) {
                        perror("accept() error ");
                    } else {
                        fprintf(stderr, "New client fd = %d\n", cfd);
                        FD_SET(cfd, &fds_backup);
                        /* Keep track of fdmax */
                        if(cfd > fdmax) {
                            fdmax = cfd;
                        }
                    }
                } else {
                    /* Got something from client */
                    n = read(i, buff, MAX_BUFF);
                    if (n == 0) {
                        fprintf(stderr, "Disconnect client fd = %d", i);
                        FD_CLR(i, &fds_backup);
                        close(i);
                        break;
                    } else if (n < 0) {
                        perror("read() error ");
                        exit(1);
                    } else {
                        /* Code handle client data */
                        
                    }
                }
            }
        }
    }
    close(socketfd);
	return 0;
}
