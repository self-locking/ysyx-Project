#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <am.h>
#include <klib-macros.h>

#define M_WORDS (1U << 24)
#define WIDTH 256
#define HEIGHT 256

uint32_t PC = 0;
uint32_t R[16] = {0};
uint32_t ROM[M_WORDS];
uint32_t RAM[M_WORDS];
int halted = 0;
static uint32_t color_buf[WIDTH * HEIGHT] = {0};

int32_t sign_extend(uint32_t imm){
	return ((int32_t)imm) >> 20;
}

void inst_cycle(){
	uint32_t inst = ROM[PC >> 2];
	uint32_t rs1 = (inst & 0xF8000) >> 15;
	uint32_t rs2 = (inst & 0x1F00000) >> 20;
	uint32_t rd = (inst & 0xF80) >> 7; 
	uint32_t opcode = inst & 0x7F;
	uint32_t func = (inst & 0x7000) >> 12;
	uint32_t imm, addr, tmp, val;

	// printf("0x%08x: %08x\n", PC, inst);

	if(opcode == 0x13 && func == 0x0){
		//addi

		imm = inst & 0xFFF00000;

		R[rd] = (rd == 0) ? 0 : (R[rs1] + sign_extend(imm));
		PC += 4;
	} 
	else if(opcode == 0x67 && func == 0x0){
		//jalr

		imm = inst & 0xFFF00000;
		uint32_t next_PC = PC + 4;
		uint32_t return_addr = (R[rs1] + sign_extend(imm)) & ~1;
		
		R[rd] = (rd == 0) ? 0 : next_PC;
		PC = return_addr;
	}
	else if(opcode == 0x33 && func == 0x0){
		//add

		R[rd] = (rd == 0) ? 0 : (R[rs1] + R[rs2]);
		PC += 4;
	}
	else if(opcode == 0x37){
		//lui

		imm = inst & 0xFFFFF000;

		R[rd] = (rd == 0) ? 0 : imm;
		PC += 4;
	}
	else if(opcode == 0x3){

		imm = (inst & 0xFFF00000);
		addr = (R[rs1] + sign_extend(imm)) >> 2;

		if(func == 0x2){
			//lw
			R[rd] = (rd == 0) ? 0 : RAM[addr];
		}
		else if(func == 0x4){
			//lbu
			tmp = RAM[addr];
			val = (R[rs1] + sign_extend(imm)) & 0x3;
			if(val == 0x0){
				R[rd] = (rd == 0) ? 0 : (tmp & 0xFF);
			}
			else if(val == 0x1){
				R[rd] = (rd == 0) ? 0 : ((tmp & 0xFF00) >> 8);
			}
			else if(val == 0x2){
				R[rd] = (rd == 0) ? 0 : ((tmp & 0xFF0000) >> 16);
			}
			else if(val == 0x3){
				R[rd] = (rd == 0) ? 0 : ((tmp & 0xFF000000) >> 24); 
			}
		}
		PC += 4;
	}
	else if(opcode == 0x23){
		
		imm = ((inst & 0xF80) << 13) + (inst & 0xFE000000);
		uint32_t byte_addr = (R[rs1] + sign_extend(imm));
		addr = byte_addr >> 2;

		if(byte_addr >= 0x20000000 && byte_addr < 0x20040000){
			int y = (byte_addr - 0x20000000) / 0x400;
            int x = ((byte_addr - 0x20000000) % 0x400) >> 2;
			color_buf[y * WIDTH + x] = R[rs2];
			PC += 4;
		}
		else{
			if(func == 0x2){
				//sw
				
				RAM[addr] = R[rs2];
			}
			else if(func == 0x0){
				//sb

				tmp = R[rs2] & 0xFF;
				val = (R[rs1] + sign_extend(imm)) & 0x3;
				if(val == 0x0){
					RAM[addr] = (RAM[addr] & 0xFFFFFF00) + tmp;
				}
				else if(val == 0x1){
					RAM[addr] = (RAM[addr] & 0xFFFF00FF) + (tmp << 8);
				}
				else if(val == 0x2){
					RAM[addr] = (RAM[addr] & 0xFF00FFFF) + (tmp << 16);
				}
				else if(val == 0x3){
					RAM[addr] = (RAM[addr] & 0x00FFFFFF) + (tmp << 24);
				}
			}
			PC += 4;
		}
	}
	else if(inst == 0x100073){
		//ebreak

		if((inst & 0xFFF00000) == 0x100000){
			printf("HIT %s TRAP\n", (R[10] == 0) ? "GOOD" : "BAD");
			halted = 1;
			return;
		}
		PC += 4;
	}
	else {PC += 4;}
}

int main(){
	ioe_init(); // initialization for GUI
	
	//const char *binfile = "mem.bin";
	const char *binfile = "vga.bin";

	FILE* fp = fopen(binfile, "rb");
	if(fp == NULL){
		perror("Error to open file");
		return 1;
	}
	//load the .bin file
	size_t n = fread(ROM, sizeof(uint32_t), M_WORDS, fp);
	fclose(fp);

	for(size_t i = 0; i < n; i++){
		RAM[i] = ROM[i];
	}

	//ROM[0x486] = 0x100073;//mem.bin
	//ROM[0x89] = 0x100073; //sum.bin
	ROM[0x36e] = 0x100073;

	uint32_t cycle = 0;	
	while(!halted){
		inst_cycle();
		cycle++;
		if(cycle % 600 == 0){
			io_write(AM_GPU_FBDRAW, 72, 22, color_buf, WIDTH, HEIGHT, true);
		}
	}
	return 0;
}

