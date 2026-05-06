#include <common.h>

typedef struct {
    char name[64];
    uint32_t addr;
    uint32_t size;
} Symbol;

static Symbol *symtab = NULL;
static int nr_sym = 0;

void init_ftrace(const char *elf_file) {
  if (elf_file == NULL) return;

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);

  // 接下来在这里写解析逻辑
  // ...
  printf("test: %s\n", elf_file);
  fclose(fp);
}

// 供以后指令执行时调用的查找函数
const char* find_func_name(uint32_t addr) {
  for (int i = 0; i < nr_sym; i++) {
    if (addr >= symtab[i].addr && addr < symtab[i].addr + symtab[i].size) {
      return symtab[i].name;
    }
  }
  return "???";
}