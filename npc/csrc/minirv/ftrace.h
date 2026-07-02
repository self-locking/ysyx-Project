#ifndef __FTRACE_H__
#define __FTRACE_H__

#include <cstdint>

// 供主激励文件调用的接口
void init_ftrace(const char *elf_file);
void free_ftrace(void);

// 声明 DPI-C 函数，防止编译器警告
extern "C" {
    void ftrace_call(uint32_t pc, uint32_t target_addr);
    void ftrace_ret(uint32_t pc);
}

#endif