#ifndef __PMEM_H__
#define __PMEM_H__

#include <cstdint>

extern "C" int  pmem_read(int raddr, int s);
extern "C" void pmem_write(int waddr, int wdata, int mask);
extern "C" void halt(int a);

void get_program_run_time(uint32_t *low, uint32_t *high);

#endif