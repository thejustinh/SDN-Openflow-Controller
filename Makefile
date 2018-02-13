CC = gcc
CFLAGS = -g -Wall -Werror

all: controller

controller: controller.c
	$(CC) $(CFLAGS) -o controller controller.c controller.h networks.c networks.h smartalloc.c smartalloc.h openflow.h -lpcap

clean:
	rm -f controller
