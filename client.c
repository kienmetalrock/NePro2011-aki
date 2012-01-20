#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

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
#define MSG_ACCEPT          13          

#define WAIT_FOR_BOARD      100
#define WAIT_FOR_PLAYER     101
#define READY_TO_PLAY       102
#define WAIT_FOR_GAMELIST   103
#define WAIT_TO_PLAY        104
#define WAIT_FOR_ACCEPT     105
#define INIT_STATUS         106

typedef struct {
    int message_id;    // type of message, define upper
    int len;
    char text[MAX_BUFF];
} msg;

int gamephase = INIT_STATUS;
int board[5][5];
int stat[5][5];
int NextNumber = 1;
double compare[26][3];

int encode_message(char * outbuff, int message_id, char * text);
int decode_message(msg * output, char * inbuff, int n);
void print_board();
void play(int s);
void gen_board();
void *updatethread();
int IsRunning = 1;
int myScore = 0;
int yourScore = 0;


int state =  -1;


int main(int argc, char* argv[]){
	int error, socketfd, i, j;
	struct addrinfo hints;
	struct addrinfo *result, *res;
	
    if (argc != 3) {
		printf("Invalid param.\n");
		exit(0);
	}
    
    for (i=0; i<5; i++) {
        for (j=0; j<5; j++) {
            stat[i][j] = 0;
        }
    }
    
    for (i=0; i<=25; i++) {
        for (j=0; j<3; j++) {
            compare[i][j] = 0;
        }
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
    
    play(socketfd);
    
    close(socketfd);
    
    return 0;
}

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
    //print_board();
}

void play(int s){
    int n;
    time_t start, end;
    fd_set fds;
    char buff[MAX_BUFF];
    char tmp[MAX_BUFF];
    msg *toreceive;
    int player, i, j, k, num;    
    double t;
    pthread_t update_thread;
    
    
    for (;;) {
        if (myScore + yourScore == 25) return;
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
                    if (gamephase == INIT_STATUS) {
                        /* send host game message */
                        gen_board();
                        memset(tmp, 0, MAX_BUFF);
                        n = encode_message(buff, MSG_HOST, tmp);
                        write(s, buff, n);
                        gamephase = WAIT_FOR_ACCEPT;
                    }
                    break;
                case 'j':
                    if (gamephase == INIT_STATUS) {
                        /* send join game message */
                        n = encode_message(buff, MSG_JOIN, tmp);
                        write(s, buff, n);
                        gamephase = WAIT_FOR_GAMELIST;
                    }
                    break;
                case 'c':
                    if (gamephase == WAIT_FOR_GAMELIST) {
                        /* Send chose game message */
                        strncpy(tmp, &buff[1], 4);
                        player = atoi(tmp);
                        printf("Join game of player %d.\n", player);
                        n = encode_message(buff, MSG_CHOSEGAME, tmp);
                        write(s, buff, n);
                        gamephase = WAIT_FOR_BOARD;
                    }
                default:
                    if (gamephase == READY_TO_PLAY) {
                        i = buff[0] - '0';
                        j = buff[1] - '0';
                        if (i >= 1 && i <=5 && j >= 1 && j <= 5) {
                            if (board[i-1][j-1] == NextNumber) {
                                if (stat[i-1][j-1] == 0) {
                                    time(&end);
                                    compare[NextNumber][0] = difftime(end, start);
                                    memset(tmp, 0, MAX_BUFF);
                                    sprintf(tmp, "%02d%08.2f", NextNumber, compare[NextNumber][0]);
                                    n = encode_message(buff, MSG_TOUCH, tmp);
                                    write(s, buff, n);
                                }
                            }
                        }
                    }
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
                        fprintf(stdout, "======= Game on server =========\n");
                        write(1, toreceive->text, toreceive->len);
                    }
                    break;
                /* a player has joined game */
                case MSG_ACCEPT:
                    if (gamephase == WAIT_FOR_ACCEPT) {
                        fprintf(stdout, "Host game successfully!\n");
                        gamephase = WAIT_FOR_PLAYER;
                    }
                    break;
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
                        memset(tmp, 0, MAX_BUFF);
                        n = encode_message(buff, MSG_READY, tmp);
                        write(s, buff, n);
                        gamephase = READY_TO_PLAY;
                    }
                    break;
                case MSG_READY:
										state=0;
                    if (gamephase == WAIT_TO_PLAY) {
                        n = encode_message(buff, MSG_START, tmp);
                        write(s, buff, n);
                        gamephase = READY_TO_PLAY;
                        time(&start);
                        fprintf(stdout, "Start !!!\n");
                        pthread_create(&update_thread, NULL, updatethread, NULL);
                        print_board();
                    }
                    break;
                case MSG_START:
										state=1;
                    fprintf(stdout, "Start !!!\n");
                    pthread_create(&update_thread, NULL, updatethread, NULL);
                    time(&start);
                    print_board();
                    break;
                case MSG_TOUCH:
                    memset(tmp, 0, MAX_BUFF);
                    strncpy(tmp, toreceive->text, 2);
                    num = atoi(tmp);
                    strncpy(tmp, &toreceive->text[2], 8);
                    t = atof(tmp);
                    compare[num][1] = t;
                    break;
                case MSG_QUIT:
                    if (myScore + yourScore < 25) fprintf(stdout, "Enemy resigned. You win !\n");
                    IsRunning = 0;
                    return;
                    break;
                default:
                    break;
            }
        }
    }    
}
void *updatethread(){
    int i, j;
    
    while (IsRunning) {
        sleep(0.01);
        if (myScore + yourScore == 25) {
            if (myScore < yourScore) fprintf(stdout, "You win !\n");
            else fprintf(stdout, "You lose !\n");
            IsRunning = 0;
            exit(0);
        }
        
        if (compare[NextNumber][0] != 0 || compare[NextNumber][1] != 0) {
            compare[NextNumber][2] = compare[NextNumber][2] + 1;
        }
        if (compare[NextNumber][2] == 100) {
            for (i=0; i<5; i++) {
                for (j=0; j<5; j++) {
                    if (board[i][j] == NextNumber) {
                        if (compare[NextNumber][0] > compare[NextNumber][1]) {
                            stat[i][j] = -1;
                            yourScore++;
                        } else {
                            stat[i][j] = 1;
                            myScore++;
                        }
                        NextNumber++;
                        print_board();
                        j = 5;
                        i = 5;
                    }
                }
            }
        }
    }
    
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
    //fprintf(stderr, "Encoded msg = $%s$\n", outbuff);
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
    
    return 0;
}

void print_board(){
    int i, j;
		fprintf(stdout,"+---+---+---+---+---+\n");
    for (i=0; i<5; i++) {
						fprintf(stdout, "| ");
        for (j=0; j<5; j++) {
						if (state ==1){
            if (stat[i][j] < 0) fprintf(stdout, "\033[01;31m%-2d\033[01;37m| ", board[i][j]);
            else if (stat[i][j] > 0) fprintf(stdout, "\033[22;34m%-2d\033[01;37m| ", board[i][j]);
            else fprintf(stdout, "\033[01;37m%-2d| ", board[i][j]);
						} else if (state ==0) {
						if (stat[i][j] > 0) fprintf(stdout, "\033[01;31m%-2d\033[01;37m| ", board[i][j]);
						else if (stat[i][j] < 0) fprintf(stdout, "\033[22;34m%-2d\033[01;37m| ", board[i][j]);
						else fprintf(stdout, "\033[01;37m%-2d| ", board[i][j]);
						}
        }
        fprintf(stdout, "\n+---+---+---+---+---+\n");
    }
    fprintf(stdout, "\033[01;37m\n");
		if (state ==0) fprintf(stdout, "YOUR COLORS: \033[01;34m BLUE\033[01;37m\n");
		else fprintf (stdout, "YOUR COLORS: \033[01;31m RED\033[01;37m\n");
    if (NextNumber <=25) fprintf(stdout, "NEXT: %d\n", NextNumber);
}
