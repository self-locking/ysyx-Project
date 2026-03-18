#include <Vex3.h>
#include <nvboard.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);

static void single_cycle() {
    dut.eval();
}

int main(int argc, char **argv)
{
	nvboard_bind_all_pins(&dut);
	nvboard_init();

	dut.eval();

	while (1)
	{
		nvboard_update();
		single_cycle();
	}

	return 0;
}
