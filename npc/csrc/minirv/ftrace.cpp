#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <elf.h>

#define Assert(cond, ...) \
  do { \
    if (!(cond)) { \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\n"); \
      exit(1); \
    } \
  } while (0)

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

  Elf32_Ehdr ehdr; 

  Assert(
    fread(&ehdr, 1, sizeof(ehdr), fp) == sizeof(ehdr), 
    "Read ELF header error"
  );

  Assert(
    memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0,
    "Not a ELF file"
  );

  // C++ 中 malloc 需要强转 (Elf32_Shdr *)
  Elf32_Shdr *shdrs = (Elf32_Shdr *)malloc(ehdr.e_shentsize * ehdr.e_shnum); 
  fseek(fp, ehdr.e_shoff, SEEK_SET); 
  Assert(
    fread(shdrs, ehdr.e_shentsize, ehdr.e_shnum, fp) == ehdr.e_shnum, 
    "Read Section header table error"
  );

  Elf32_Shdr shstr = shdrs[ehdr.e_shstrndx];
  
  char *shstrtab = (char *)malloc(shstr.sh_size);
  fseek(fp, shstr.sh_offset, SEEK_SET); 
  Assert(
    fread(shstrtab, 1, shstr.sh_size, fp) == shstr.sh_size, 
    "Read .shstrtab error"
  );

  Elf32_Shdr symtab_hdr = {0};
  Elf32_Shdr strtab_hdr = {0};
  for (int i = 0; i < ehdr.e_shnum; i++) {
    char *name = shstrtab + shdrs[i].sh_name;
    if (strcmp(name, ".symtab") == 0) { symtab_hdr = shdrs[i]; }
    if (strcmp(name, ".strtab") == 0) { strtab_hdr = shdrs[i]; }
  }

  char *strtab = (char *)malloc(strtab_hdr.sh_size);
  fseek(fp, strtab_hdr.sh_offset, SEEK_SET);
  Assert(
    fread(strtab, 1, strtab_hdr.sh_size, fp) == strtab_hdr.sh_size, 
    "Read .strtab error"
  );
  
  int sym_count = symtab_hdr.sh_size / sizeof(Elf32_Sym);
  Elf32_Sym *syms = (Elf32_Sym *)malloc(symtab_hdr.sh_size);
  fseek(fp, symtab_hdr.sh_offset, SEEK_SET);
  Assert(
    fread(syms, sizeof(Elf32_Sym), sym_count, fp) == sym_count, 
    "Read .symtab error"
  );

  symtab = (Symbol *)malloc(sizeof(Symbol) * sym_count);
  for (int i = 0; i < sym_count; i++) {
    Elf32_Sym *sym = &syms[i];
    if (ELF32_ST_TYPE(sym->st_info) != STT_FUNC ||
        sym->st_size == 0 ||
        sym->st_value == 0 ||
        sym->st_name == 0
      ) {continue;} 
    char *func_name = strtab + sym->st_name; 
    strncpy(symtab[nr_sym].name, func_name, sizeof(symtab[nr_sym].name) - 1);
    symtab[nr_sym].name[sizeof(symtab[nr_sym].name) - 1] = '\0';
    symtab[nr_sym].addr = sym->st_value;
    symtab[nr_sym].size = sym->st_size;
    nr_sym++;
  }

  enable_ftrace = 1;
  free(shdrs);
  free(shstrtab);
  free(strtab);
  free(syms);
  fclose(fp);
}

const char* find_func_name(uint32_t addr) {
  for (int i = 0; i < nr_sym; i++) {
    if (addr >= symtab[i].addr && addr < symtab[i].addr + symtab[i].size) {
      return symtab[i].name;
    }
  }
  return "???";
}

extern "C" void ftrace_call(int pc, int target_addr) {
  if (!enable_ftrace) return;
  const char *func_name = find_func_name(target_addr);
  printf("0x%08x: ", pc);
  for(int i = 0; i < depth; i++) printf("  ");
  printf("call [%s@0x%08x]\n", func_name, target_addr);
  depth++;
}

extern "C" void ftrace_ret(int pc) {
  if (!enable_ftrace) return;
  depth--;
  if (depth < 0) depth = 0; // 安全边界保护
  const char *func_name = find_func_name(pc);
  printf("0x%08x: ", pc);
  for(int i = 0; i < depth; i++) printf("  ");
  printf("ret [%s]\n", func_name);
}

void free_ftrace(void) {
  if(symtab != NULL) {
    free(symtab);
    symtab = NULL;
  }
}