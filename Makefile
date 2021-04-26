# MITROI EDUARD IONUT 312CA
CC=clang
CFLAGS=-Wall -Wextra -std=c99

build: tema2 clean

tema2: main.o Hashtable.o LinkedList.o load_balancer.o server.o Extra.o
	$(CC) $(CFLAGS) -Wall -Wextra main.o Hashtable.o LinkedList.o load_balancer.o server.o Extra.o -o tema2

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

Hashtable.o: Hashtable.c
	$(CC) $(CFLAGS) -c Hashtable.c

LinkedList.o: LinkedList.c
	$(CC) $(CFLAGS) -c LinkedList.c

load_balancer.o: load_balancer.c
	$(CC) $(CFLAGS) -c load_balancer.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

Extra.o: Extra.c
	$(CC) $(CFLAGS) -c Extra.c

clean:
	rm -f *.o

