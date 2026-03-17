TOPNAME = ex7
NXDC_FILES = constr/ex7.nxdc
INC_PATH ?=

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD -build -cc  \
				-Wall -O3 --x-assign fast --x-initial fast --noassert -Wno-DECLFILENAME -Wno-UNUSEDSIGNAL

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
SIM_BIN = $(OBJ_DIR)/V$(TOPNAME)

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = ./csrc/auto_bind.cpp
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
VSRCS = ./vsrc/ex7.v
VSRCS += ./vsrc/my_tool/*.v
CSRCS = ./csrc/sim_ex_clk.cpp
CSRCS += $(SRC_AUTO_BIND)

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

all: default sim

sim: $(SIM_BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./$(SIM_BIN)

run: $(BIN)
	@$^

clean:
	rm -rf $(BUILD_DIR) $(SRC_AUTO_BIND) 

.PHONY: default all clean run sim

include ../Makefile

# # 生成波形
# # 配置
# TOP_MODULE      := top                       # 你的 Verilog 顶层模块名
# SIM_TOP_CPP     := csrc/sim_top.cpp          # C++ 仿真入口
# V_SOURCES       := vsrc/top.v                # .v文件入口
# OBJ_DIR         := obj_dir
# SIM_BIN         := $(OBJ_DIR)/V$(TOP_MODULE) # 编译后的可执行文件目录
# WAVE_DIR        := wave_logs
# WAVE_FILE       := $(WAVE_DIR)/top_wave.fst  # 波形文件路径

# # 确保波形目录存在
# $(shell mkdir -p $(WAVE_DIR))

# .PHONY: all sim clean

# all: sim

# sim: $(SIM_BIN)
# 	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# 	./$(SIM_BIN)
# 	@echo "Stimulation finished."

# # 编译规则，--trace-fst开启波形输出，-j多线程加速编译，-Who*忽略一些Verilator警告
# $(SIM_BIN): $(SIM_TOP_CPP) $(V_SOURCES) | $(WAVE_DIR)
# 	verilator --cc --exe --build -j \
# 	--Mdir $(OBJ_DIR) \
# 	--top-module $(TOP_MODULE) \
# 	--trace-fst \
# 	-Wall \
# 	-Wno-fatal -Wno-lint \
# 	-o V$(TOP_MODULE) \
# 	$(SIM_TOP_CPP) \
# 	$(V_SOURCES)

# $(WAVE_DIR):
# 	mkdir -p $@

# clean:
# 	rm -rf $(OBJ_DIR) $(WAVE_DIR)/*.fst *.log

# include ../Makefile
