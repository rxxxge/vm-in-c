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

uint16_t sign_extend(uint16_t x, int bit_count);
void update_flags(uint16_t r);

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

// TRAP codes
enum {
	TRAP_GETC 	= 0x20,
	TRAP_OUT 	= 0x21,
	TRAP_PUTS 	= 0x22,
	TRAP_IN 	= 0x23,
	TRAP_PUTSP 	= 0x24,
	TRAP_HALT 	= 0x25
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
	// Handle user input
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
			// destination register (DR)
			uint16_t r0 = (instr >> 9) & 0x7;
			// first operand (SR1)
			uint16_t r1 = (instr >> 6) & 0x7;
			// whether we are in immediate mode
			uint16_t imm_flag = (instr >> 5) & 0x1;

			if (imm_flag) {
				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0] = reg[r1] + imm5;
			} else {
				uint16_t r2 = instr & 0x7;
				reg[r0] = reg[r1] + reg[r2];
			}

			update_flags(r0);

			break;
		case OP_AND:
			// destination register (DR)
			uint16_t r0 = (instr >> 9) & 0x7;
			// first operand (SR1)
			uint16_t r1 = (instr >> 6) & 0x7;
			uint16_t and_flag = (instr >> 5) & 0x1;

			if (and_flag) {
				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0] = reg[r1] & imm5;
			} else {
				uint16_t r2 = instr & 0x7;
				reg[r0] = reg[r1] & reg[r2];
			}
			update_flags(r0);

			break;
		case OP_NOT:
			// (DR)
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t r1 = (instr >> 6) & 0x7;
			reg[r0] = ~reg[r1];
			update_flags(r0);

			break;
		case OP_BR:
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			uint16_t cond_flag = (instr >> 9) & 0x7;
			if (cond_flag & reg[R_COND]) 
				reg[R_PC] += pc_offset;
			break;
		case OP_JMP:
			uint16_t base_r = (instr >> 6) & 0x7;
			reg[R_PC] = reg[base_r];
			break;
		case OP_JSR:
			uint16_t long_flag = (instr >> 11) & 1;

			reg[R_R7] = reg[R_PC];
			if (long_flag) {
				uint16_t long_offset = sign_extend(instr & 0x7FF, 11);
				reg[R_PC] += long_offset;
			} else {
				uint16_t base_r = (instr >> 6) & 0x7;
				reg[R_PC] = reg[base_r];
			}

			break;
		case OP_LD:
			// DR
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t ld_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] += mem_read(reg[R_PC] + ld_offset);
			update_flags(r0);

			break;
		case OP_LDI:
			// Destination register (DR)
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t ldi_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] = mem_read(mem_read(reg[R_PC] + ldi_offset));
			update_flags(r0);

			break;
		case OP_LDR:
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t base_ldr = (instr >> 6) & 0x7;
			uint16_t ldr_offset = sign_extend(instr & 0x3F, 6);
			reg[r0] = mem_read(reg[base_ldr] + ldr_offset);
			update_flags(r0);

			break;
		case OP_LEA:
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t lea_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] = reg[R_PC] + lea_offset;
			update_flags(r0);

			break;
		case OP_ST:
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t st_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(reg[R_PC] + st_offset, reg[r0]); 
			break;
		case OP_STI:
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t sti_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(mem_read(reg[R_PC] + sti_offset), reg[r0]);
			break;
		case OP_STR:
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t base_str = (instr >> 6) & 0x7;
			uint16_t str_offset = sign_extend(instr & 0x3F, 6);
			mem_write(reg[base_str] + str_offset, reg[r0]);

			break;
		case OP_TRAP:
			reg[R_R7] = reg[R_PC];

			switch (instr & 0xFF) {
			case TRAP_GETC:
				// TODO
				break;
			case TRAP_OUT:
				// TODO
				break;
			case TRAP_PUTS:
				// TODO
				break;
			case TRAP_IN:
				// TODO
				break;
			case TRAP_PUTSP:
				// TODO
				break;
			case TRAP_HALT:
				// TODO
				break;
			}

			break;
		case OP_RES:
		case OP_RTI:
		default:
			abort();
			break;
		}
	}
}

// Sign Extend
uint16_t sign_extend(uint16_t x, int bit_count) {
	if ((x >> (bit_count - 1)) & 1) {
		x |= (0xFFFF << bit_count);
	}

	return x;
}

void update_flags(uint16_t r) {
	if (reg[r] == 0) {
		reg[R_COND] = FL_ZRO;
	} else if (reg[r] >> 15) {
		reg[R_COND] = FL_NEG;
	} else {
		reg[R_COND] = FL_POS;
	}
}