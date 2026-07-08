#include <cstdio>
#include <cassert>
#include <cstdint>
#include <capstone/capstone.h>
#include "itrace.h"

static csh capstone_handle;
static FILE *itrace_log_fp = nullptr;

void init_disasm() {
    if (cs_open(CS_ARCH_RISCV, CS_MODE_RISCV32, &capstone_handle) != CS_ERR_OK) {
        printf("Error: Failed to initialize Capstone disassembler!\n");
        exit(1);
    }

    itrace_log_fp = fopen("npc_log.txt", "w");
    if (itrace_log_fp == nullptr) {
        perror("Error opening itrace.log");
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
    if (itrace_log_fp != nullptr) {
        fclose(itrace_log_fp);
        itrace_log_fp = nullptr;
    }
}