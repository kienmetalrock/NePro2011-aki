all: server client

client: client.o
	gcc -pthread -o client client.o
client.o: client.c
	gcc -c client.c
    
server: server.o
	gcc -o server server.o
server.o: server.c
	gcc -c server.c
