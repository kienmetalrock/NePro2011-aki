CC     = gcc
CFLAGS = `pkg-config --cflags gtk+-2.0 gmodule-2.0`
LIBS   = `pkg-config --libs   gtk+-2.0 gmodule-2.0`
DEBUG  = -Wall -g

OBJECTS = main.o callbacks.o

.PHONY: clean

all: main server

server: server.o
	gcc -o server server.o
server.o: server.c
	gcc -c server.c

main: $(OBJECTS)
	$(CC) $(DEBUG) $(LIBS) $(OBJECTS) -o $@

main.o: main.c support.h
	$(CC) $(DEBUG) $(CFLAGS) -c $< -o $@

callbacks.o: callbacks.c support.h
	$(CC) $(DEBUG) $(CFLAGS) -c $< -o $@

clean:
	rm *.o

