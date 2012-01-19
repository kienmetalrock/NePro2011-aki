#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MAX_CONN 128
#define MAX_BUFF 1024
#define MAX_CLIENT 100 // equvalent to max pair

#define MSG_PLAYERJOIN      1
#define MSG_DISCONNECT      2
#define MSG_REFUSE          3
#define MSG_GAMELIST        4
#define MSG_CHOSEGAME       5
#define MSG_READY           6
#define MSG_TOUCH           7
#define MSG_START           8       
#define MSG_QUIT            9
#define MSG_HOST            10
#define MSG_JOIN            11
#define MSG_BOARD           12

typedef struct {
    int message_id;    // type of message, define upper
    int len;
    char text[MAX_BUFF];
} msg;

int encode_message(char * outbuff, int message_id, char * text);
int decode_message(msg * output, char * inbuff, int n);

int main(int argc, char* argv[]){
	int error, socketfd, cfd;
    char hostname[256];
	struct addrinfo hints;
	struct addrinfo *result, *res;
	struct sockaddr peer_sockaddr;
	socklen_t peer_sockaddr_size;
	char port[] = "12345";
	char buff[MAX_BUFF];
    char tmp[MAX_BUFF];
    int i, n, j;
    fd_set fds, fds_backup;
    int fdmax;
    msg *toreceive;
    int client[MAX_CLIENT];
    
    
    /* Init variable for client array*/
    for (i = 0; i<MAX_CLIENT; i++) {
        /* -2 mean that there is no client with id = i*/ 
        client[i] = -2;
    }

    gethostname(hostname, sizeof(hostname));
    fprintf(stderr, "hostname = %s\n", hostname);
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
                    if (cfd >= MAX_CLIENT) {
                        memset(buff, 0, MAX_BUFF);
                        strcpy(buff, "Full Client. Connection refused !");
                        write(i, buff, strlen(buff)+1);
                        fprintf(stderr, "New client fd = %d\n", cfd);
                        fprintf(stderr, "Full Client. Connection refused !");
                        close(i);
                    }
                    if (cfd == -1) {
                        perror("accept() error ");
                    } else {
                        fprintf(stderr, "New client fd = %d\n", cfd);
                        client[i] = -1;
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
                        fprintf(stderr, "Disconnect client fd = %d\n", i);
                        FD_CLR(i, &fds_backup);
                        close(i);
                        break;
                    } else if (n < 0) {
                        perror("read() error ");
                        exit(1);
                    } else {
                        /* Code handle client data */
                        toreceive = (msg *) malloc(sizeof(msg));
                        decode_message(toreceive, buff, n);
                        fprintf(stderr, "Message from fd = %d\n", i);
                        fprintf(stderr, "Decoded msg: message_id = %d, len = %d, text = %s\n", toreceive->message_id, toreceive->len, toreceive->text);
                        memset(buff, 0, MAX_BUFF);
                        switch (toreceive->message_id) {
                            case MSG_HOST:
                                fprintf(stdout, "Player %d host game.\n", i);
                                /* Set client i to hosting */
                                client[i] = 0;
                                break;
                            case MSG_JOIN:
                                fprintf(stdout, "Player %d want join a game. Send game list.\n", i);
                                /* Create game list */                                
                                for (j=0; j< MAX_CLIENT; j++) {
                                    memset(tmp, 0, MAX_BUFF);
                                    if (client[j] == 0) {
                                        sprintf(tmp, "Game %02d, Player 1/2\n", j);                                        
                                        strcat(buff, tmp);
                                    }
                                }
                                strncpy(tmp, buff, MAX_BUFF);
                                n = encode_message(buff, MSG_GAMELIST, tmp);
                                write(i, buff, n);
                                break;
                            case MSG_CHOSEGAME:
                                j = atoi(toreceive->text);
                                fprintf(stdout, "Player %d want join game of player %d. Notify player.\n", i, j);
                                /* Set pair of player */
                                client[i] = j;
                                client[j] = i;
                                memset(tmp, 0, MAX_BUFF);
                                n = encode_message(buff, MSG_PLAYERJOIN, tmp);
                                write(j, buff, n);
                                break;
                            case MSG_READY:
                            case MSG_BOARD:
                            case MSG_START:
                            case MSG_TOUCH:
                                if (client[i] > 0) {
                                    j = client[i];
                                    n = encode_message(buff, toreceive->message_id, toreceive->text);
                                    write(j, buff, n);
                                } else fprintf(stdout, "Unknown target client.\n");
                                break;
                            default:
                                break;
                        }
                        free(toreceive);
                    }
                }
            }
        }
    }
    close(socketfd);
	return 0;
}
/* return encoded buff and its length */
int encode_message(char * outbuff, int message_id, char * text){
    int len;
    char tmp[5];
    memset(outbuff, 0, MAX_BUFF);
    sprintf(tmp, "%04d", message_id);
    memcpy(&outbuff[0], tmp, 4);
    if (strlen(text) == 0) {
        len = 0;
    } else {
        len = strlen(text) + 1;
        memcpy(&outbuff[8], text, len);
    }
    sprintf(tmp, "%04d", len);
    memcpy(&outbuff[4], tmp, 4);    
    fprintf(stderr, "Encoded msg = $%s$\n", outbuff);
    return 8 + len;
}

int decode_message(msg * output, char * inbuff, int n){
    char tmp[5];
    
    memset(output->text, 0, MAX_BUFF);
    tmp[4] = '\0';
    memcpy(tmp, &inbuff[0], 4); 
    output->message_id = atoi(tmp);
    memcpy(tmp, &inbuff[4], 4);
    output->len = atoi(tmp);
    memcpy(output->text, &inbuff[8], output->len);
    
    fprintf(stderr, "Decoded msg: message_id = %d, len = %d, text = %s\n", output->message_id, output->len, output->text);
    return 0;
}
