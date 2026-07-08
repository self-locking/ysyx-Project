#ifndef __MTRACE_H__
#define __MTRACE_H__

#include <cstdio>
#include <cstdint>

// header-only：不受 CONFIG_MTRACE 控制时函数体为空，编译器会直接优化掉，
// 因此不需要在 Makefile 里额外追加 .cpp 编译单元。

static inline void mtrace_read(uint32_t pc, uint32_t addr, uint32_t data) {
#ifdef CONFIG_MTRACE
    printf("0x%08x: Read: [0x%08x] %08x\n", pc, addr, data);
#endif
}

static inline void mtrace_write(uint32_t pc, uint32_t addr, uint32_t data, int mask) {
#ifdef CONFIG_MTRACE
    printf("0x%08x: Write:%08x-->[0x%08x], mask=%d]\n", pc, data, addr, mask);
#endif
}

#endif