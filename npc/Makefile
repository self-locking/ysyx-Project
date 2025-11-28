# 配置
TOP_MODULE      := top                       # 你的 Verilog 顶层模块名
SIM_TOP_CPP     := csrc/sim_top.cpp          # C++ 仿真入口
V_SOURCES       := vsrc/top.v                # .v文件入口
OBJ_DIR         := obj_dir
SIM_BIN         := $(OBJ_DIR)/V$(TOP_MODULE) # 编译后的可执行文件目录 
WAVE_DIR        := wave_logs
WAVE_FILE       := $(WAVE_DIR)/top_wave.fst  # 波形文件路径

# 确保波形目录存在
$(shell mkdir -p $(WAVE_DIR))

.PHONY: all sim clean

all: sim

sim: $(SIM_BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./$(SIM_BIN)
	@echo "Stimulation finished."

# 编译规则，--trace-fst开启波形输出，-j多线程加速编译，-Who*忽略一些Verilator警告
$(SIM_BIN): $(SIM_TOP_CPP) $(V_SOURCES) | $(WAVE_DIR)
	verilator --cc --exe --build -j \
		--Mdir $(OBJ_DIR) \
		--top-module $(TOP_MODULE) \
		--trace-fst \
		-Wall \
		-Wno-fatal -Wno-lint \
		-o V$(TOP_MODULE) \
		$(SIM_TOP_CPP) \
		$(V_SOURCES)

$(WAVE_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(WAVE_DIR)/*.fst *.log

include ../Makefile
