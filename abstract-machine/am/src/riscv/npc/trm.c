#include <am.h>
#include <klib-macros.h>
#include <stdio.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

#define SERIAL_PORT (*(volatile uint8_t *)0x10000000)

void putch(char ch) {
  SERIAL_PORT = ch;
}

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : :"r"(code)); // 告诉编译器不要优化，原封不动写入

  while (1);
}

void _trm_init() {
  uint32_t ysyx;
  uint32_t id;

  asm volatile("csrr %0, mvendorid" : "=r"(ysyx));
  asm volatile("csrr %0, marchid" : "=r"(id));

  printf("ID = %c%c%c%c_%d\n",
      (ysyx >> 24) & 0xff,
      (ysyx >> 16) & 0xff,
      (ysyx >> 8)  & 0xff,
      ysyx & 0xff,
      id);
  
  int ret = main(mainargs);
  halt(ret);
}
