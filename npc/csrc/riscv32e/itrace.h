#ifndef __ITRACE_H__
#define __ITRACE_H__

void init_disasm();
void free_disasm();

extern "C" void disassemble_and_log(int pc, int insn);

#endif