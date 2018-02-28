CC = gcc
CFLAGS = -g -Wall -Werror -lpcap -pthread

all: controller

controller: controller.c
	$(CC) $(CFLAGS) -o controller controller.c controller.h networks.c networks.h smartalloc.c smartalloc.h openflow.h myGraph.c myGraph.h

clean:
	rm -f controller
