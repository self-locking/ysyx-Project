#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include "difftest.h"

static void (*ref_difftest_memcpy)(uint32_t, void *, size_t, bool);
static void (*ref_difftest_regcpy)(void *, bool);
static void (*ref_difftest_exec)(uint64_t);
static void (*ref_difftest_init)(int);


void init_difftest(const char *ref_so_file, void *img, size_t img_size, CPU_state *cpu) {
    void *handle = dlopen(ref_so_file, RTLD_LAZY);
    assert(handle);

    ref_difftest_memcpy =
        (decltype(ref_difftest_memcpy))
        dlsym(handle, "difftest_memcpy");
    assert(ref_difftest_memcpy);

    ref_difftest_regcpy =
        (decltype(ref_difftest_regcpy))
        dlsym(handle, "difftest_regcpy");
    assert(ref_difftest_regcpy);

    ref_difftest_exec =
        (decltype(ref_difftest_exec))
        dlsym(handle, "difftest_exec");
    assert(ref_difftest_exec);

    ref_difftest_init =
        (decltype(ref_difftest_init))
        dlsym(handle, "difftest_init");
    assert(ref_difftest_init);

    ref_difftest_init(0);

    ref_difftest_memcpy(0x80000000, img, img_size, DIFFTEST_TO_REF);
    ref_difftest_regcpy(cpu, DIFFTEST_TO_REF);

}

void difftest_step(CPU_state *npc_cpu) {
    CPU_state ref_cpu;
    ref_difftest_exec(1);

    ref_difftest_regcpy(&ref_cpu, DIFFTEST_TO_DUT);

    checkregs(&ref_cpu, npc_cpu);
}

void checkregs(CPU_state *ref, CPU_state *dut) {
    // if (ref->pc == dut->pc) {
    //     for (int i = 0; i < 16; i++) { // 目前只有16个reg
    //         if (ref->gpr[i] != dut->gpr[i]) {
    //             printf("x%d is different: ref: 0x%08x, dut: 0x%08x\n", i, ref->gpr[i], dut->gpr[i]);
    //             assert(0);
    //         }
    //     }
    // } else {
    //     printf("PC is different: ref: 0x%08x, dut: 0x%08x\n", ref->pc, dut->pc);
    //     assert(0);
    // }
}