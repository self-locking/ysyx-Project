# 配置
TOP_MODULE      := minirv                       # Verilog 顶层模块名
SIM_CPPS        += csrc/minirv/sim_minirv.cpp   # C++ 仿真入口
V_SOURCES       := vsrc/minirv/*.v              # .v文件入口
OBJ_DIR         := obj_dir
SIM_BIN         := $(OBJ_DIR)/V$(TOP_MODULE)    # 编译后的可执行文件目录
WAVE_DIR        := wave_logs
WAVE_FILE       := $(WAVE_DIR)/minirv_wave.fst  # 波形文件路径
IMG             ?=                               # 二进制文件 (.bin)

# ==========================================================
CONFIG_ITRACE 	?= y
CONFIG_MTRACE 	?= n
CONFIG_FTRACE 	?= n
CONFIG_DIFFTEST ?= y
# ==========================================================

# 如果传入了 IMG (xxx.bin)，通过 patsubst 将其后缀替换为 .elf 
ELF_FILE :=
ifneq ($(IMG),)
    ELF_FILE := $(patsubst %.bin,%.elf,$(IMG))
endif

# 确保波形目录存在
$(shell mkdir -p $(WAVE_DIR))

.PHONY: all sim clean

all: sim

sim: $(SIM_BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./$(SIM_BIN) $(IMG) $(ELF_FILE)
	@echo "Stimulation finished."

SIM_CFLAGS  := -I$(NEMU_HOME)/tools/capstone/repo/include
SIM_LDFLAGS := -lreadline

ifeq ($(CONFIG_DIFFTEST), y)
	SIM_CFLAGS += -DCONFIG_DIFFTEST
	SIM_LDFLAGS += -ldl
	SIM_CPPS += csrc/minirv/difftest.cpp
endif

# 根据选择追加参数
ifeq ($(CONFIG_ITRACE), y)
    # 传给 Verilog 的宏，用于 RTL 内部 `ifdef ITRACE 控制 DPI-C
    VERILATOR_USER_FLAGS += -DITRACE=1
    
    # 传给 C++ 的宏，用于 cpp 激励文件中的#ifdef CONFIG_ITRACE
    SIM_CFLAGS += -DCONFIG_ITRACE
    
    # 链接 Capstone 动态库的路径和名称
    SIM_LDFLAGS += -L$(NEMU_HOME)/tools/capstone/repo -lcapstone
endif

ifeq ($(CONFIG_MTRACE), y)
    SIM_CFLAGS += -DCONFIG_MTRACE
endif

ifeq ($(CONFIG_FTRACE), y)
    VERILATOR_USER_FLAGS += -DFTRACE=1
    SIM_CFLAGS += -DCONFIG_FTRACE
    SIM_CPPS += csrc/minirv/ftrace.cpp
endif


# 编译规则（注意：$(SIM_TOP_CPP) 已被替换为支持多文件的 $(SIM_CPPS)）
$(SIM_BIN): $(SIM_CPPS) $(V_SOURCES) | $(WAVE_DIR)
	verilator --cc --exe --build -j \
	--Mdir $(OBJ_DIR) \
	--top-module $(TOP_MODULE) \
	--trace-fst \
	-Wall \
	-Wno-fatal -Wno-lint \
	$(VERILATOR_USER_FLAGS) \
	-CFLAGS "$(SIM_CFLAGS)" \
	-LDFLAGS "$(SIM_LDFLAGS)" \
	-o V$(TOP_MODULE) \
	$(SIM_CPPS) \
	$(V_SOURCES)

$(WAVE_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(WAVE_DIR)/*.fst *.log

include ../Makefile