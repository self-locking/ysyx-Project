#include <stdio.h>
#include <cstdint>
#include <readline/readline.h>
#include <sys/time.h> // 获取系统时间
#include <verilated.h>
#include "Vriscv32e.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "Vriscv32e__Dpi.h"
#include "Vriscv32e___024root.h" // 用于访问寄存器和PC的值
#include "ftrace.h"
#include "difftest.h"

#ifdef CONFIG_ITRACE
#include <capstone/capstone.h> // 用于反汇编
csh capstone_handle;
FILE *itrace_log_fp = nullptr;

void init_disasm() {
    if (cs_open(CS_ARCH_RISCV, CS_MODE_RISCV32, &capstone_handle) != CS_ERR_OK) {
        printf("Error: Failed to initialize Capstone disassembler!\n");
        exit(1);
    }
}

// 反汇编并记录一条指令
extern "C" void disassemble_and_log(int pc, int insn) {
    cs_insn *insn_decompressed;
    // 将 32 位指令整型转为字节数组
    uint8_t code[4];
    code[0] = insn & 0xFF;
    code[1] = (insn >> 8) & 0xFF;
    code[2] = (insn >> 16) & 0xFF;
    code[3] = (insn >> 24) & 0xFF;

    // 执行反汇编
    size_t count = cs_disasm(capstone_handle, code, 4, pc, 0, &insn_decompressed);
    if (count > 0) {
        if (itrace_log_fp != nullptr) {
            fprintf(itrace_log_fp, "0x%08x: 0x%08x   %s\t%s\n", 
                    pc, insn, insn_decompressed[0].mnemonic, insn_decompressed[0].op_str);

            fflush(itrace_log_fp); // 刷新缓冲区
        
        }
        // 释放内存
        cs_free(insn_decompressed, 1);
    } else {
        assert(0 && "Failed to disassemble instruction");
    }
}

// 在仿真结束时释放 Capstone 资源
void free_disasm() {
    cs_close(&capstone_handle);
}

#endif

#define WAVE_GEN

#define MAX_SIZE 0x2000000
#define MBASE 0x20000000
#define SERIAL_PORT 0x10000000
#define RTC_ADDR_LOW 0x02004000
#define RTC_ADDR_HIGH 0x02004004



static int is_running = 1;
static int cpu_running = 1;
static uint32_t pmem[MAX_SIZE];

static Vriscv32e* top = new Vriscv32e;

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
    while(n-- && cpu_running) {
        if(i) {
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

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }
  line_read = readline("(npc) ");

  return line_read;

}

static int cmd_q(char *args) {
  is_running = 0;
  return 0;
}

static int cmd_c(char *args) {
    if (!cpu_running) {
        printf("Program has terminated.\n");
        return 0;
    }
    cpu_exec(-1);

    return 0;
}

static int cmd_si(char *args) {
    if (!cpu_running) {
        printf("Program has terminated.\n");
        return 0;
    }
    
    int n = 1;
    if (args != NULL) {
        n = atoi(args);
    }
    cpu_exec(n);

    return 0;
}

static int cmd_pc(char *args) {
    uint32_t pc = top->PC;
    printf("0x%08x: %08x\n", pc, pmem[(pc >> 2) - MBASE]);
    
    return 0;
}

static int cmd_info(char *args);

static int cmd_x(char *args);

// sdb指令
static struct {
    const char *name;
    int (*handler) (char *);
} cmd_table [] = {
    {"c", cmd_c},
    {"si", cmd_si},
    {"info", cmd_info},
    {"x", cmd_x},
    {"q", cmd_q},
    {"pc", cmd_pc},
};

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

#ifdef CONFIG_MTRACE
    if (s) printf("0x%08x: Read: [0x%08x] %08x\n", top->PC, u_raddr, pmem[u_raddr - MBASE]);
#endif

    return pmem[u_raddr - MBASE];
}

extern "C" void pmem_write(int waddr, int wdata, int mask) {
    uint8_t ch = uint8_t(wdata & 0xFF);
    uint32_t paddr = waddr - MBASE;
    uint32_t data = pmem[paddr];

#ifdef CONFIG_MTRACE
    printf("0x%08x: Write:%08x-->[0x%08x], mask=%d]\n", top->PC, wdata, waddr, mask);
#endif

    if ((waddr << 2) == SERIAL_PORT) {putchar(ch); return;};
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
extern "C" void halt (int a) {
    cpu_running = 0;
    if (a) {
        printf("\nnpc: \033[1;31mHIT BAD TRAP\033[0m\n");
    }
    else printf("\nnpc: \33[1;32mHIT GOOD TRAP\33[0m\n");
    return;
}

static int cmd_info(char *args) {
    if (args == NULL) {
        printf("Please input the subcmd\n");
    }
    else if (strcmp(args, "r") == 0) {
        for(int i = 0; i < 16; i++) {
            printf("x%02d = 0x%x\n", i, top->rootp->riscv32e__DOT__gpr7__DOT__rf[i]);
        }
    }
    return 0;
}

static int cmd_x(char *args) {
    int n = 0;
    uint32_t addr = 0;

    if (args == NULL) {
        printf("Unknown subcmd\n");
        return 0;
    }

    // 解析两个参数
    int ret = sscanf(args, "%d %x", &n, &addr);
    if (ret != 2 || n <= 0) {
        printf("Unknown subcmd\n");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        uint32_t cur = addr / 4;
        if (cur < MBASE || cur - MBASE >= MAX_SIZE) {
            printf("out of bound\n");
            continue;
        }

        uint32_t data = pmem[(cur - MBASE)];
        printf("0x%08x: %08x\n", addr, data);
        addr += 4;
    }

    return 0;
}

int main (int argc, char **argv) {
    /*====================== 初始化disasm =====================*/
#ifdef CONFIG_ITRACE
    init_disasm();
    itrace_log_fp = fopen("npc_log.txt", "w");
    if (itrace_log_fp == nullptr) {
        perror("Error opening itrace.log");
        return 1;
    }
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
    tfp->open("wave_logs/minirv_wave.fst"); // 写入波形文件，文件地址和文件名可以自定义
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
    while (is_running) {  
        for (char *str; is_running && ((str = rl_gets()) != NULL); ) {
            char *str_end = str + strlen(str);
            char *cmd = strtok(str, " ");

            if (cmd == NULL) { continue; }

            char *args = cmd + strlen(cmd) + 1;
            if (args >= str_end) {
                args = NULL;
            }

            int i;
            for (i = 0; i < 6; i ++) {
                if (strcmp(cmd, cmd_table[i].name) == 0) {
                    if (cmd_table[i].handler(args) < 0) { printf("Wrong\n"); return 0; }
                    break;
                }
            }
            if (i == 6)  printf("Unknown command '%s'\n", cmd);
        }
    }

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
    if (itrace_log_fp != nullptr) {
        fclose(itrace_log_fp);
        itrace_log_fp = nullptr;
    }
#endif

#ifdef CONFIG_FTRACE
    free_ftrace();
#endif  

    return 0;
}