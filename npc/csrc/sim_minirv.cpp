#include <stdio.h>
#include <verilated.h>
#include "Vminirv.h"
#include "verilated_fst_c.h"
#include "svdpi.h"
#include "Vminirv__Dpi.h"

// 用于采样
void step_and_dump(Vminirv* top, VerilatedFstC* tfp, VerilatedContext* contextp) {
    top->eval();
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

// ebreak实现
static int is_running = 1;
extern "C" void halt (int a) {
    is_running = 0;
    if (a) {
        printf("\n\33[1;31mHIT BAD TRAP\33[0m\n");
    }
    else printf("\n\33[1;32mHIT GOOD TRAP\33[0m\n");
    return;
}

int main (int argc, char **argv) {
    VerilatedContext *contextp = new VerilatedContext;// contextp用来保存仿真的时间
    contextp->commandArgs(argc, argv);                // 传递参数给verilator
    Vminirv* top = new Vminirv{contextp};
    
    //打开波形
	VerilatedFstC *tfp = new VerilatedFstC; // 创建一个波形文件指针
	contextp->traceEverOn(true);            // 启用跟踪
	top->trace(tfp, 99);                    // 采样深度为99(是否采集子模块等)
	tfp->open("wave_logs/minirv_wave.fst");    // 写入波形文件，文件地址和文件名可以自定义

    // 初始化信号并稳定路
	top->rst = 1;
    top->clk = 0;
    step_and_dump(top, tfp, contextp);
    top->clk = 1;
    step_and_dump(top, tfp, contextp);
	top->rst = 0;

    // int cycle = 7; // 执行n-1条指令
    while (is_running) {
        top->clk = 0;
        step_and_dump(top, tfp, contextp);
        top->clk = 1;
        step_and_dump(top, tfp, contextp);
    }

	// 清理top仿真模型，并销毁相关指针，并将指针变为空指针
	tfp->close();  // 关闭波形文件
	top->final();  // 让Verilator进行清理工作
	delete top;
	top = nullptr;
	delete contextp;
	contextp = nullptr;

	return 0;
}

