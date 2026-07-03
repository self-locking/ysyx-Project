#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum { DIFFTEST_TO_DUT = 0, DIFFTEST_TO_REF = 1 };

typedef struct {
    uint32_t gpr[32];
    uint32_t pc;
} CPU_state;

void init_difftest(const char *ref_so, void *img, size_t img_size, CPU_state *cpu);
void difftest_step(CPU_state *npc_cpu);
void checkregs(CPU_state *ref, CPU_state *dut);

#endif