#include <stdio.h>
#include <cstdint>
#include <sys/time.h> // 获取系统时间
#include <verilated.h>
#include "Vminirv.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "Vminirv__Dpi.h"

#define MAX_SIZE 0x2000000
#define MBASE 0x20000000
#define SERIAL_PORT 0x10000000
#define RTC_ADDR_LOW 0x02004000
#define RTC_ADDR_HIGH 0x02004004

static int is_running = 1;
static uint32_t pmem[MAX_SIZE];

void get_program_run_time(uint32_t *low, uint32_t *high) {
    static struct timespec start_time;
    static int first = 1;
    struct timespec current_time;
    
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    if (first) {
        start_time = current_time;
        first = 0;
    }

    uint64_t diff_sec = current_time.tv_sec - start_time.tv_sec;
    long diff_nsec = current_time.tv_nsec - start_time.tv_nsec;

    // 处理纳秒借位的情况（比如当前 100ns，启动时 200ns，不够减）
    if (diff_nsec < 0) {
        diff_sec  -= 1;
        diff_nsec += 1000000000L;
    }

    uint64_t run_us = (diff_sec * 1000000ULL) + (diff_nsec / 1000);

    *low  = (uint32_t)(run_us & 0xFFFFFFFFu);
    *high = (uint32_t)((run_us >> 32) & 0xFFFFFFFFu);
}

extern "C" int pmem_read(int raddr) {
    static uint32_t low = 0;
    static uint32_t high = 0;
    uint32_t u_raddr = (uint32_t) raddr;

    if ((u_raddr << 2) == RTC_ADDR_HIGH) {
        get_program_run_time(&low, &high);
        return high;
    }
    else if ((u_raddr << 2) == RTC_ADDR_LOW) return low;

    if (u_raddr < MBASE || u_raddr - MBASE >= MAX_SIZE) {
        // out of bound
        return 0;
    }

    return pmem[u_raddr - MBASE];
}

extern "C" void pmem_write(int waddr, int wdata, int mask) {
    uint8_t data = uint8_t(wdata & 0xFF);
    uint32_t paddr = waddr - MBASE;
    if ((waddr << 2) == SERIAL_PORT) {putchar(data); return;};
    
    switch (mask)
    {
    case 0:
        pmem[paddr] = wdata;
        break;
    case 1:
        pmem[paddr] = (pmem[paddr] & ~0x000000FF) | data;
        break;
    case 2:
        pmem[paddr] = (pmem[paddr] & ~0x0000FF00) | (data << 8);
        break;
    case 3:
        pmem[paddr] = (pmem[paddr] & ~0x00FF0000) |(data << 16);
        break;
    case 4:
        pmem[paddr] = (pmem[paddr] & ~0xFF000000) | (data << 24);
        break;
    default:
        break;
    }
}

// ebreak实现
extern "C" void halt (int a) {
    is_running = 0;
    if (a) {
        printf("\n\33[1;31mHIT BAD TRAP\33[0m\n");
    }
    else printf("\n\33[1;32mHIT GOOD TRAP\33[0m\n");
    return;
}

// // 用于debug
// extern "C" void regPrint(const svBitVecVal* regs) {
//     for(int i = 0; i < 16; i++) {
//         printf("x%d: %08x\n", i, (uint32_t)regs[i]);
//     }
// }

// 用于采样
void step_and_dump(Vminirv* top, VerilatedFstC* tfp, VerilatedContext* contextp) {
    top->eval();
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

int main (int argc, char **argv) {
    
    VerilatedContext *contextp = new VerilatedContext;// contextp用来保存仿真的时间
    contextp->commandArgs(argc, argv);                // 传递参数给verilator
    Vminirv* top = new Vminirv{contextp};
    
    //打开波形
	VerilatedFstC *tfp = new VerilatedFstC; // 创建一个波形文件指针
	contextp->traceEverOn(true);            // 启用跟踪
	top->trace(tfp, 99);                    // 采样深度为99(是否采集子模块等)
	tfp->open("wave_logs/minirv_wave.fst");    // 写入波形文件，文件地址和文件名可以自定义

    // 写入数据
    // const char *binfile = "./resource/sum.bin";
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

    size_t n = fread(pmem, sizeof(uint32_t), MAX_SIZE, fp);
    fclose(fp);

    // pmem[0x486] = 0x100073; // mem.bin
    // pmem[0x89] = 0x100073; // sum.bin

    // 初始化信号并稳定路
	top->rst = 1;
    top->clk = 0;
    step_and_dump(top, tfp, contextp);
    top->clk = 1;
    step_and_dump(top, tfp, contextp);
	top->rst = 0;

    // int cycle = 0; 
    while (is_running) {
        top->clk = 0;
        step_and_dump(top, tfp, contextp);
        top->clk = 1;
        step_and_dump(top, tfp, contextp);
        // printf("%08x\n", top->cp-4);
        // cycle++;
    }

    // printf("%d\n", cycle);

	// 清理top仿真模型，并销毁相关指针，并将指针变为空指针
	tfp->close();  // 关闭波形文件
	top->final();  // 让Verilator进行清理工作
	delete top;
	top = nullptr;
	delete contextp;
	contextp = nullptr;

	return 0;
}

