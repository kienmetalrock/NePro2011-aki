#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAX_BUFF 1024

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

#define WAIT_FOR_BOARD      100
#define WAIT_FOR_PLAYER     101
#define READY_TO_PLAY       102
#define WAIT_FOR_GAMELIST   103
#define WAIT_TO_PLAY        104

typedef struct {
    int message_id;    // type of message, define upper
    int len;
    char text[MAX_BUFF];
} msg;

int encode_message(char * outbuff, int message_id, char * text);
int decode_message(msg * output, char * inbuff, int n);
int gamephase;
int board[5][5];

void gen_board(){
    int i, j, k;
    int status[26];
    
    status[0] = 0;
    k = 1;
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            board[i][j] = 0;
            status[k] = 0;
            k++;
        }
    }
    
    srand(time(NULL));    
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            k = rand()%25 + 1;
            while (status[k] == 1) k = rand()%25 + 1;
            board[i][j] = k;
            status[k] = 1;
        }
    }
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            fprintf(stdout, "%2d ", board[i][j]);
        }
        fprintf(stdout, "\n");
    }
}

void chat(int s){
    int n;
    fd_set fds;
    char buff[MAX_BUFF];
    char tmp[MAX_BUFF];
    msg *toreceive;
    int player, i, j, k;    
    
    for (;;) {
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(s, &fds);
        select(s+1, &fds, NULL, NULL, NULL);
        if (FD_ISSET(0, &fds)) {
            n = read(0, buff, MAX_BUFF);
            if (n == 0) break;
            buff[n] = '\0';
            memset(tmp, 0, MAX_BUFF);
            switch (buff[0]) {
                case 'h':
                    /* send host game message */
                    gen_board();
                    memset(tmp, 0, MAX_BUFF);
                    n = encode_message(buff, MSG_HOST, tmp);
                    write(s, buff, n);
                    gamephase = WAIT_FOR_PLAYER;
                    break;
                case 'j':
                    /* send join game message */
                    n = encode_message(buff, MSG_JOIN, tmp);
                    write(s, buff, n);
                    gamephase = WAIT_FOR_GAMELIST;
                    break;
                case 'c':
                    /* Send chose game message */
                    strncpy(tmp, &buff[1], 4);
                    player = atoi(tmp);
                    printf("Join game of player %d.\n", player);
                    n = encode_message(buff, MSG_CHOSEGAME, tmp);
                    write(s, buff, n);
                    gamephase = WAIT_FOR_BOARD;
                    break;
            }
        }
        if (FD_ISSET(s, &fds)) {
            n = read(s, buff, 512);
            if (n == 0) break;
            toreceive = (msg *) malloc(sizeof(msg));
            decode_message(toreceive, buff, n);
            memset(tmp, 0, MAX_BUFF);
            memset(buff,0, MAX_BUFF);
            switch (toreceive->message_id) {
                /* received game list */
                case MSG_GAMELIST:
                    if (gamephase == WAIT_FOR_GAMELIST) {
                        write(1, toreceive->text, toreceive->len);
                    }
                    break;
                /* a player has joined game */
                case MSG_PLAYERJOIN:
                    if (gamephase == WAIT_FOR_PLAYER) {
                        fprintf(stdout, "Player joined. Send board now.\n");
                        for (i=0; i<5; i++) {
                            for (j=0; j<5; j++) {
                                sprintf(tmp, "%02d", board[i][j]);
                                strcat(buff, tmp);
                            }
                        }
                        memset(tmp, 0, MAX_BUFF);
                        strncpy(tmp, buff, MAX_BUFF);
                        n = encode_message(buff, MSG_BOARD, tmp);
                        write(s, buff, n);
                        gamephase = WAIT_TO_PLAY;
                    }
                    break;
                /* receive game board */
                case MSG_BOARD:
                    if (gamephase == WAIT_FOR_BOARD) {
                        fprintf(stdout, "Got board data.\n");
                        k = 0;
                        for (i=0; i<5; i++) {
                            for (j=0; j<5; j++) {
                                strncpy(tmp, &toreceive->text[k], 2);
                                board[i][j] = atoi(tmp);
                                k += 2;
                            }
                        }
                        for (i=0; i<5; i++) {
                            for (j=0; j<5; j++) {
                                fprintf(stdout, "%2d ", board[i][j]);
                            }
                            fprintf(stdout, "\n");
                        }
                        memset(tmp, 0, MAX_BUFF);
                        n = encode_message(buff, MSG_READY, tmp);
                        write(s, buff, n);
                        gamephase = READY_TO_PLAY;
                    }
                    break;
                case MSG_READY:
                    if (gamephase == WAIT_TO_PLAY) {
                        n = encode_message(buff, MSG_START, tmp);
                        write(s, buff, n);
                        gamephase = READY_TO_PLAY;
                        fprintf(stdout, "Start !!!\n");
                    }
                    break;
                case MSG_START:
                    fprintf(stdout, "Start !!!\n");
                    break;
                case MSG_TOUCH:
                    break;
                default:
                    break;
            }
        }
    }    
}

int main(int argc, char* argv[]){
	int error, socketfd;
	struct addrinfo hints;
	struct addrinfo *result, *res;
	
    if (argc != 3) {
		printf("Invalid param.\n");
		exit(0);
	}
	printf("Host address: %s\nPost: %s\n", argv[1], argv[2]);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((error = getaddrinfo(argv[1], argv[2], &hints, &result))){
		fprintf(stderr, "getaddrinfo(%s, %s, %p, %p)  got error %s ", argv[1], argv[2], &hints, &result, gai_strerror(error));
		exit(-1);
	}

	for (res = result; res != NULL; res = res->ai_next) {
		socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketfd == -1)
			continue;

		if (connect(socketfd, res->ai_addr, res->ai_addrlen) != -1) {
			printf("Connect Successfully !\n");
			break;
		}

		close(socketfd);
	}
    
    if (res == NULL) {
        printf("Couldn't connect.\n");
        exit(-1);
    }
    
    freeaddrinfo(result);
    
    chat(socketfd);
    
    /*
    for (;;) {
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(socketfd, &fds);
        select(socketfd+1, &fds, NULL, NULL, NULL);
        if (FD_ISSET(0, &fds)) {
            n = read(0, buff, 512);
            if (n == 0) break;
            write(socketfd, buff, n);
        }
        if (FD_ISSET(socketfd, &fds)) {
            n = read(socketfd, buff, 512);
            if (n == 0) break;
            write(1, buff, n);
        }
    }   
    */
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
