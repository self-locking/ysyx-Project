#include <cstdio>
#include <ctime>
#include "pmem.h"
#include "mtrace.h"
#include "npc.h"

uint32_t pmem[MAX_SIZE];

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

extern "C" int pmem_read(int raddr, int s) {
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

    if (s) mtrace_read(top->PC, u_raddr, pmem[u_raddr - MBASE]);

    return pmem[u_raddr - MBASE];
}

extern "C" void pmem_write(int waddr, int wdata, int mask) {
    uint8_t ch = uint8_t(wdata & 0xFF);
    uint32_t u_waddr = (uint32_t) waddr;
 
    if ((u_waddr << 2) == SERIAL_PORT) { putc(ch, stderr); return; }
 
    if (u_waddr < MBASE || u_waddr - MBASE >= MAX_SIZE) {
        printf("0x%08x: pmem_write out of bound! waddr=0x%08x wdata=0x%08x mask=%d\n",
               top->PC, u_waddr, wdata, mask);
        return;
    }
 
    uint32_t paddr = u_waddr - MBASE;
    uint32_t data = pmem[paddr];
 
    mtrace_write(top->PC, waddr, wdata, mask);


    switch (mask)
    {
    case 1:
        pmem[paddr] = (data & ~0x000000FF) | wdata;
        break;
    case 2:
        pmem[paddr] = (data & ~0x0000FF00) | (wdata << 8);
        break;
    case 3:
        pmem[paddr] = (data & ~0x0000FFFF) | wdata;
        break;
    case 4:
        pmem[paddr] = (data & ~0x00FF0000) |(wdata << 16);
        break;
    case 8:
        pmem[paddr] = (data & ~0xFF000000) | (wdata << 24);
        break;
    case 10:
        pmem[paddr] = (data & ~0xFFFF0000) | (wdata << 16);
        break;
    case 15:
        pmem[paddr] = wdata;
        break;
    default:
        break;
    }
}

// ebreak实现
extern "C" void halt(int a) {
    cpu_running = 0;
    if (a) {
        printf("npc: \033[1;31mHIT BAD TRAP\033[0m\n");
    }
    else printf("npc: \33[1;32mHIT GOOD TRAP\33[0m\n");
    return;
}