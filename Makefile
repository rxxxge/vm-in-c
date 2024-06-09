CC=gcc

vm: vm.c main.c
	$(CC) vm.c main.c -o vm