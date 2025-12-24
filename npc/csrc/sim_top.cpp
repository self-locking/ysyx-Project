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
