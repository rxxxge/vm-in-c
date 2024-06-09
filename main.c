#include "vm.h"


int main(int argc, const char *argv[]) {
	// Handle user input
	if (argc < 2) {
		printf("vm [image-file1] ...\n");
		exit(2);
	}

	for (int j = 1; j < argc; ++j) {
		if (!read_image(argv[j])) {
			printf("Failed to load image: %s\n", argv[j]);
			exit(1);
		}
	}

	signal(SIGINT, handle_interrupt);
	disable_input_buffering();

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
			uint16_t r0_add = (instr >> 9) & 0x7;
			// first operand (SR1)
			uint16_t r1_add = (instr >> 6) & 0x7;
			// whether we are in immediate mode
			uint16_t imm_flag = (instr >> 5) & 0x1;

			if (imm_flag) {
				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0_add] = reg[r1_add] + imm5;
			} else {
				uint16_t r2 = instr & 0x7;
				reg[r0_add] = reg[r1_add] + reg[r2];
			}

			update_flags(r0_add);

			break;
		case OP_AND:
			// destination register (DR)
			uint16_t r0_and = (instr >> 9) & 0x7;
			// first operand (SR1)
			uint16_t r1_and = (instr >> 6) & 0x7;
			uint16_t and_flag = (instr >> 5) & 0x1;

			if (and_flag) {
				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0_and] = reg[r1_and] & imm5;
			} else {
				uint16_t r2 = instr & 0x7;
				reg[r0_and] = reg[r1_and] & reg[r2];
			}
			update_flags(r0_and);

			break;
		case OP_NOT:
			// (DR)
			uint16_t r0_not = (instr >> 9) & 0x7;
			uint16_t r1_not = (instr >> 6) & 0x7;
			reg[r0_not] = ~reg[r1_not];
			update_flags(r0_not);

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
			uint16_t r0_ld = (instr >> 9) & 0x7;
			uint16_t ld_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0_ld] += mem_read(reg[R_PC] + ld_offset);
			update_flags(r0_ld);

			break;
		case OP_LDI:
			// Destination register (DR)
			uint16_t r0_ldi = (instr >> 9) & 0x7;
			uint16_t ldi_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0_ldi] = mem_read(mem_read(reg[R_PC] + ldi_offset));
			update_flags(r0_ldi);

			break;
		case OP_LDR:
			uint16_t r0_ldr = (instr >> 9) & 0x7;
			uint16_t base_ldr = (instr >> 6) & 0x7;
			uint16_t ldr_offset = sign_extend(instr & 0x3F, 6);
			reg[r0_ldr] = mem_read(reg[base_ldr] + ldr_offset);
			update_flags(r0_ldr);

			break;
		case OP_LEA:
			uint16_t r0_lea = (instr >> 9) & 0x7;
			uint16_t lea_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0_lea] = reg[R_PC] + lea_offset;
			update_flags(r0_lea);

			break;
		case OP_ST:
			uint16_t r0_st = (instr >> 9) & 0x7;
			uint16_t st_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(reg[R_PC] + st_offset, reg[r0_st]); 
			break;
		case OP_STI:
			uint16_t r0_sti = (instr >> 9) & 0x7;
			uint16_t sti_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(mem_read(reg[R_PC] + sti_offset), reg[r0_sti]);
			break;
		case OP_STR:
			uint16_t r0_str = (instr >> 9) & 0x7;
			uint16_t base_str = (instr >> 6) & 0x7;
			uint16_t str_offset = sign_extend(instr & 0x3F, 6);
			mem_write(reg[base_str] + str_offset, reg[r0_str]);
			break;
		case OP_TRAP:
			reg[R_R7] = reg[R_PC];

			switch (instr & 0xFF) {
			case TRAP_GETC:
				reg[R_R0] = (uint16_t)getchar();
				update_flags(R_R0);
				break;
			case TRAP_OUT:
				putc((char)reg[R_R0], stdout);
				fflush(stdout);
				break;
			case TRAP_PUTS:
				uint16_t *c = memory + reg[R_R0];
				while (*c) {
					putc((char)*c, stdout);
					++c;
				}
				fflush(stdout);
				break;
			case TRAP_IN:
				printf("Enter a character: ");
				char ch = getchar();
				putc(ch, stdout);
				fflush(stdout);
				reg[R_R0] = (uint16_t)ch;
				update_flags(R_R0);
				break;
			case TRAP_PUTSP:
				uint16_t *ch_t = memory + reg[R_R0];
				while (*ch_t) {
					char char1 = (*ch_t) & 0xFF;
					putc(char1, stdout);
					char char2 = (*ch_t) >> 8;
					if (char2) putc(char2, stdout);
					++c;
				}
				fflush(stdout);
				break;
			case TRAP_HALT:
				puts("HALT");
				fflush(stdout);
				running = 0;
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
	restore_input_buffering();
}
