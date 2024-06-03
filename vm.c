#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

// Registers
enum {
	R_R0 = 0,
	R_R1,
	R_R2,
	R_R3,
	R_R4,
	R_R5,
	R_R6,
	R_R7,
	R_PC, // program counter
	R_COND,
	R_COUNT
};

// Condition flags
enum {
	FL_POS = 1 << 0,
	FL_ZRO = 1 << 1,
	FL_NEG = 1 << 2
};

// Opcodes
enum {
	OP_BR = 0, /* branch */
	OP_ADD,	   /* add  */
	OP_LD,	   /* load */
	OP_ST,	   /* store */
	OP_JSR,	   /* jump register */
	OP_AND,	   /* bitwise and */
	OP_LDR,	   /* load register */
	OP_STR,	   /* store register */
	OP_RTI,	   /* unused */
	OP_NOT,	   /* bitwise not */
	OP_LDI,	   /* load indirect */
	OP_STI,	   /* store indirect */
	OP_JMP,	   /* jump */
	OP_RES,	   /* reserved (unused) */
	OP_LEA,	   /* load effective address */
	OP_TRAP	   /* execute trap */
};

// Memory storage
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; // 65536 locations

// Register storage
uint16_t reg[R_COUNT];

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		printf("vm [image-file1] ...\n");
		exit(2);
	}

	for (int j = 1; j < argc; ++j) {
		if (!read_image[argv[j]]) {
			printf("Failed to load image: %s\n", argv[j]);
			exit(1);
		}
	}

	reg[R_PC] = FL_ZRO;
	
	enum { PC_START = 0x3000 };
	reg[R_PC] = PC_START;

	int running = 1;
	while (running) {
		// Fetch
		uint16_t instr = mem_read(reg[R_PC]++);
		uint16_t op = instr >> 12;

		switch (op) {
		case OP_ADD:
			// TODO
			break;
		case OP_AND:
			// TODO
			break;
		case OP_NOT:
			// TODO
			break;
		case OP_BR:
			// TODO
			break;
		case OP_JMP:
			// TODO
			break;
		case OP_JSR:
			// TODO
			break;
		case OP_LD:
			// TODO
			break;
		case OP_LDI:
			// TODO
			break;
		case OP_LDR:
			// TODO
			break;
		case OP_LEA:
			// TODO
			break;
		case OP_ST:
			// TODO
			break;
		case OP_STI:
			// TODO
			break;
		case OP_STR:
			// TODO
			break;
		case OP_TRAP:
			// TODO
			break;
		case OP_RES:
		case OP_RTI:
		default:
			// TODO
			break;
		}
	}
}