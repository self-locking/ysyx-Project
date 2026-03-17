// 使用verilator必须include
#include <Vtop.h>  // 仿真模型的头文件，由top.v生成，如果顶层文件名更改则也需要更改

// 调用NVBoard的API
#include <nvboard.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);

// 用于状态更新
static void single_cycle() {
    dut.clk = 0;
    dut.eval();
    dut.clk = 1;
    dut.eval();
}

//仿真的过程
int main(int argc, char **argv)
{
// 链接引脚并初始化	
	nvboard_bind_all_pins(&dut);
	nvboard_init();

// 初始化信号并稳定路
	dut.clk = 0;
	dut.rst = 0;
	dut.f = 1;
	dut.eval();

	while (1)
	{
		nvboard_update();
		single_cycle();
	}

	return 0;
}

/* ===============生成波形========================= */

// // 与verilator无关的头文件
// #include <stdio.h>

// // 使用verilator必须include
// #include "Vtop.h"  // 仿真模型的头文件，由top.v生成，如果顶层文件名更改则也需要更改
// #include <verilated.h>
// #include "verilated_fst_c.h" // 波形文件所需的头文件

// //仿真的过程
// int main(int argc, char **argv)
// {
// 	VerilatedContext *contextp = new VerilatedContext;// contextp用来保存仿真的时间
// 	contextp->commandArgs(argc, argv);                // 传递参数给verilator

// 	Vtop *top = new Vtop{contextp};                   // 构建一个名为top的仿真模型

// 	//打开波形
// 	VerilatedFstC *tfp = new VerilatedFstC; // 创建一个波形文件指针
// 	contextp->traceEverOn(true);                // 启用跟踪
// 	top->trace(tfp, 99);                                // 采样深度为99(是否采集子模块等)
// 	tfp->open("wave_logs/top_wave.fst");    // 写入波形文件，文件地址和文件名可以自定义

// 	//对top信号的初始化
// 	top->rst = 0;

// 	int cycle = 50; //定义仿真循环次数，即仿真50个循环后就自动结束
// 	while (cycle)
// 	{
// 		/************设置vtop仿真的输入信号************/
// 		int a = rand() & 1;
// 		int b = rand() & 1;
// 		top->a = a;
// 		top->b = b;
// 		top->eval();          //更新电路状态,一定要写
// 		printf("a = %d, b = %d, f = %d\n", a, b, top->f);   //打印内容
// 		contextp->timeInc(1); //推动仿真时间

// 		tfp->dump(contextp->time()); // 按照时间采样

// 		cycle--;
//     }

// 	// 清理top仿真模型，并销毁相关指针，并将指针变为空指针
// 	tfp->close();  // 关闭波形文件
// 	top->final();  // 让Verilator进行清理工作
// 	delete top;
// 	top = nullptr;
// 	delete contextp;
// 	contextp = nullptr;

// 	return 0;
// }