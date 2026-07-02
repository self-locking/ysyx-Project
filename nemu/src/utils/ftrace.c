#include <common.h>
#include <elf.h>

typedef struct {
    char name[64];
    uint32_t addr;
    uint32_t size;
} Symbol;

static Symbol *symtab = NULL;
static int nr_sym = 0;
static int depth = 0;
static int enable_ftrace = 0;

void init_ftrace(const char *elf_file) {
  if (elf_file == NULL) return;

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);

  Elf32_Ehdr ehdr; // elf文件的Header

  Assert(
    fread(&ehdr, 1, sizeof(ehdr), fp) == sizeof(ehdr), 
    "Read ELF header error"
  );

  Assert(
    // 判断头部中魔数是否符合elf文件形式
    // e_ident 为标识数组，ELFMAG 是ELF文件的标识，SELFMAG 定义为4
    memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0,
    "Not a ELF file"
  );

  Elf32_Shdr *shdrs = malloc(ehdr.e_shentsize * ehdr.e_shnum); // 分配节头表存储空间
  fseek(fp, ehdr.e_shoff, SEEK_SET); // 将指针移动到文件开头偏移e_shoff处，即节头表
  Assert(
    // 读取节头表
    fread(shdrs, ehdr.e_shentsize, ehdr.e_shnum, fp) == ehdr.e_shnum, 
    "Read Section header table error"
  );

  // 取出.shstrtab 的 section header
  // 节头表名称存放于.shstrtab中，而索引又存放于 ehdr.e_shstrndx中
  Elf32_Shdr shstr = shdrs[ehdr.e_shstrndx];
  
  char *shstrtab = malloc(shstr.sh_size);
  fseek(fp, shstr.sh_offset, SEEK_SET); // 移动到对应节区
  Assert(
    // 读取.shstrtab(格式是形如\0.symtab\0.strtab\0.shstrtab\0)
    fread(shstrtab, 1, shstr.sh_size, fp) == shstr.sh_size, 
    "Read .shstrtab error"
  );

  Elf32_Shdr symtab_hdr = {0};
  Elf32_Shdr strtab_hdr = {0};
  // 遍历寻找 .symtab 和 .strtab，前者记录索引地址，后者记录字符串地址
  for (int i = 0; i < ehdr.e_shnum; i++) {

    char *name = shstrtab + shdrs[i].sh_name;

    if (strcmp(name, ".symtab") == 0) {
        symtab_hdr = shdrs[i];
    }

    if (strcmp(name, ".strtab") == 0) {
        strtab_hdr = shdrs[i];
    }
  }

  // 读取 .strtab
  char *strtab = malloc(strtab_hdr.sh_size);
  fseek(fp, strtab_hdr.sh_offset, SEEK_SET);
  Assert(
    fread(strtab, 1, strtab_hdr.sh_size, fp) == strtab_hdr.sh_size, 
    "Read .strtab error"
  );
  
  // 读取 .symtab
  int sym_count = symtab_hdr.sh_size / sizeof(Elf32_Sym);
  Elf32_Sym *syms = malloc(symtab_hdr.sh_size);
  fseek(fp, symtab_hdr.sh_offset, SEEK_SET);
  Assert(
    fread(syms, sizeof(Elf32_Sym), sym_count, fp) == sym_count, 
    "Read .symtab error"
  );

  symtab = malloc(sizeof(Symbol) * sym_count);
  // 遍历寻找 FUNC 并填入
  for (int i = 0; i < sym_count; i++) {
    Elf32_Sym *sym = &syms[i];
    if (ELF32_ST_TYPE(sym->st_info) != STT_FUNC ||
        sym->st_size == 0 ||
        sym->st_value == 0 ||
        sym->st_name == 0
      ) {continue;} 
    char *func_name = strtab + sym->st_name; // st_name 记录的是偏移
    strncpy(symtab[nr_sym].name, func_name, sizeof(symtab[nr_sym].name) - 1);
    symtab[nr_sym].name[sizeof(symtab[nr_sym].name) - 1] = '\0';
    symtab[nr_sym].addr = sym->st_value;
    symtab[nr_sym].size = sym->st_size;
    nr_sym++;
  }

  enable_ftrace = 1;
  // printf("test: %s\n", elf_file);
  free(shdrs);
  free(shstrtab);
  free(strtab);
  free(syms);
  fclose(fp);
}

// 供以后指令执行时调用的查找函数
const char* find_func_name(vaddr_t addr) {
  for (int i = 0; i < nr_sym; i++) {
    if (addr >= symtab[i].addr && addr < symtab[i].addr + symtab[i].size) {
      return symtab[i].name;
    }
  }
  return "???";
}

void ftrace_call(vaddr_t pc, vaddr_t target_addr) {
  if (!enable_ftrace) return;
  const char *func_name = find_func_name(target_addr);
  printf("0x%08x: ",pc);
  for(int i = 0; i < depth; i++) printf(" ");
  printf("call [%s@0x%08x]\n", func_name, target_addr);
  depth++;
}

void ftrace_ret(vaddr_t pc) {
  if (!enable_ftrace) return;
  depth--;
  const char *func_name = find_func_name(pc);
  printf("0x%08x: ",pc);
  for(int i = 0; i < depth; i++) printf(" ");
  printf("ret [%s]\n", func_name);
}

void free_ftrace(void) {
  if(symtab != NULL) {
    free(symtab);
    symtab = NULL;
  }
}