#ifndef VM_H
#define VM_H


#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#ifdef WIN32

#include <Windows.h>
#include <conio.h> // _kbhit

#else // unix

#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#endif

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

enum {
	MR_KBSR = 0xFE00,	// keyboard status
	MR_KBDR = 0xFE02	// keyboard data	
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
extern uint16_t memory[MEMORY_MAX]; // 65536 locations

// Register storage
extern uint16_t reg[R_COUNT];

uint16_t sign_extend(uint16_t x, int bit_count);
uint16_t swap16(uint16_t x);
void update_flags(uint16_t r);
int read_image_file(FILE* file);
int read_image(const char* image_path);
void mem_write(uint16_t address, uint16_t val);
uint16_t mem_read(uint16_t address);

#ifdef WIN32

extern HANDLE hStdin;
extern DWORD fdwMode, fdwOldMode;

#else

extern struct termios original_tio;

#endif

void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();

void handle_interrupt(int signal);


#endif // VM_H