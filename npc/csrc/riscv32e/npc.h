#ifndef __NPC_H__
#define __NPC_H__

#include <cstdint>
#include "Vriscv32e.h"
#include "difftest.h"

#define MAX_SIZE      0x2000000
#define MBASE         0x20000000
#define SERIAL_PORT   0x10000000
#define RTC_ADDR_LOW  0x02004000
#define RTC_ADDR_HIGH 0x02004004

extern Vriscv32e *top;

// 物理内存，pmem.cpp 中定义
extern uint32_t pmem[MAX_SIZE];

extern int is_running;
extern int cpu_running;

void step_and_dump();
void cpu_exec(uint64_t n);
void get_npc_cpu(CPU_state *cpu);

#endif