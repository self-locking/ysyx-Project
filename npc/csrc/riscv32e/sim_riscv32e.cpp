#include <stdio.h>
#include <cstdint>
#include <verilated.h>
#include "Vriscv32e.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "Vriscv32e__Dpi.h"
#include "Vriscv32e___024root.h" // 用于访问寄存器和PC的值

#include "npc.h"
#include "sdb.h"
#include "pmem.h"

#ifdef CONFIG_ITRACE
#include "itrace.h"
#endif

#ifdef CONFIG_FTRACE
#include "ftrace.h"
#endif

#ifdef CONFIG_DIFFTEST
#include "difftest.h"
#endif

// #define WAVE_GEN
#define BATCH_MODE

int is_running = 1;
int cpu_running = 1;

Vriscv32e *top = new Vriscv32e;

#ifdef WAVE_GEN
static VerilatedContext *contextp = nullptr;
static VerilatedFstC *tfp = nullptr;
#endif

// 用于更新和采样
void step_and_dump() {
    top->eval();
#ifdef WAVE_GEN
    contextp->timeInc(1);
    tfp->dump(contextp->time());
#endif
}

// 获取 npc 的状态
void get_npc_cpu(CPU_state *cpu) {
    for (int i = 0; i < 16; i++) {
        cpu->gpr[i] = top->rootp->riscv32e__DOT__gpr7__DOT__rf[i];
    }
    for (int i = 16; i < 32; i++) cpu->gpr[i] = 0;

    cpu->pc = top->PC;
}

// 指令执行（单周期）
void cpu_exec(uint64_t n) {
    int i = 0;
    if (n <= 10 && n > 0) i = 1;
    while (n-- && cpu_running) {
        if (i) {
            uint32_t pc = top->PC;
            uint32_t inst = pmem[(pc >> 2) - MBASE];
            printf("0x%08x: 0x%08x\n", pc, inst);
        }
        top->clk = 0;
        step_and_dump();
        top->clk = 1;
        step_and_dump();

#ifdef CONFIG_DIFFTEST
        CPU_state npc_cpu;
        get_npc_cpu(&npc_cpu);
        difftest_step(&npc_cpu);
#endif
    }
}

int main(int argc, char **argv) {
    /*====================== 初始化disasm =====================*/
#ifdef CONFIG_ITRACE
    init_disasm();
#endif

    /*====================== 初始化ftrace =====================*/
#ifdef CONFIG_FTRACE
    const char *elf_file = (argc > 2) ? argv[2] : nullptr;
    init_ftrace(elf_file);
#endif

    /*====================== 打开波形 =====================*/
#ifdef WAVE_GEN
    contextp = new VerilatedContext;        // contextp用来保存仿真的时间
    contextp->commandArgs(argc, argv);      // 传递参数给verilator
    top = new Vriscv32e{contextp};

    tfp = new VerilatedFstC;                // 创建一个波形文件指针
    contextp->traceEverOn(true);            // 启用跟踪
    top->trace(tfp, 99);                    // 采样深度为99(是否采集子模块等)
    tfp->open("wave_logs/riscv32e_wave.fst"); // 写入波形文件，文件地址和文件名可以自定义
#endif

    /*====================== 写入数据 =====================*/
    const char *binfile = argc > 1 ? argv[1] : nullptr;
    if (binfile == nullptr) {
        perror("\33[1;31mLack of binfile name\33[0m\n");
        return 1;
    }

    FILE *fp = fopen(binfile, "rb");
    if (fp == nullptr) {
        perror("Error to open the file\n");
        return 1;
    }

    size_t n = fread(pmem, 1, MAX_SIZE * sizeof(uint32_t), fp);
    fclose(fp);

    /*====================== 初始化difftest =====================*/
#ifdef CONFIG_DIFFTEST
    CPU_state npc_cpu;
    get_npc_cpu(&npc_cpu);
    npc_cpu.pc = 0x80000000; // 设置初始PC为0x80000000
    init_difftest("../nemu/build/riscv32-nemu-interpreter-so", pmem, n, &npc_cpu);
#endif

    /*====================== 初始化信号并稳定电路 =====================*/
    top->rst = 1;

    for (int i = 0; i < 10; i++) {
        top->clk = 0;
        step_and_dump();
        top->clk = 1;
        step_and_dump();
    }

    top->rst = 0;

    /*====================== 运行 =====================*/
#ifdef BATCH_MODE
    cpu_exec(-1);
#else
    sdb_mainloop();
#endif
    /*====================== 清理 =====================*/
    top->final();  // 让Verilator进行清理工作
    delete top;
    top = nullptr;

#ifdef WAVE_GEN
    tfp->close();
    delete contextp;
    contextp = nullptr;
#endif

#ifdef CONFIG_ITRACE
    free_disasm();
#endif

#ifdef CONFIG_FTRACE
    free_ftrace();
#endif

    return 0;
}
