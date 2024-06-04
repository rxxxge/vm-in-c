CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic

vm: vm.c
	$(CC) $(CFLAGS) -o vm vm.c