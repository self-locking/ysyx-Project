module IDU (
    input wire [31:0] src1,
    input wire [31:0] src2,
    input wire [31:0] inst,
    input wire [31:0] pc,
    output wire [4:0] rs1_addr,
    output wire [4:0] rs2_addr,
    output wire [4:0] rd_addr,
    output wire wen,
    output wire [9:0] exu_op, // can add more bits
    output wire [31:0] data1,
    output wire [31:0] data2,
    output wire [31:0] data3,
    output wire [31:0] data4,
    output wire [3:0] lsu_op,
    output wire [1:0] wbu_op
);

wire [6:0] opcode;
wire [2:0] func3;
wire [6:0] func7;

wire r_type;
wire i_type;
wire s_type;
wire u_type;
wire j_type;
wire b_type;

wire [31:0] imm;
wire [31:0] i_imm;
wire [31:0] s_imm;
wire [31:0] u_imm;
wire [31:0] j_imm;
wire [31:0] b_imm;

wire inst_addi;
wire inst_sltiu;
wire inst_jalr;
wire inst_add;
wire inst_sub;
wire inst_lui;
wire inst_auipc;
wire inst_lw;
wire inst_lbu;
wire inst_sw;
wire inst_sb;
wire inst_beq;
wire inst_bne;
wire inst_ebreak;

/* ========= ready =========== */
assign opcode = inst[6:0];
assign func3  = inst[14:12];
assign func7  = inst[31:25];

assign r_type = (opcode == 7'h33);
assign i_type = (opcode == 7'h13) | (opcode == 7'h3) | (opcode == 7'h67);
assign s_type = (opcode == 7'h23);
assign u_type = (opcode == 7'h37) | (opcode == 7'h17);
assign j_type = (opcode == 7'h6F);
assign b_type = (opcode == 7'h63);

assign i_imm = { {20{inst[31]}}, inst[31:20] };
assign s_imm = { {20{inst[31]}}, inst[31:25], inst[11:7] };
assign u_imm = { inst[31:12], {12{1'b0}} };
assign j_imm = { {12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0 };
assign b_imm = { {20{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0 };
assign imm =    ({32{i_type}} & i_imm) | 
                ({32{s_type}} & s_imm) | 
                ({32{u_type}} & u_imm) | 
                ({32{j_type}} & j_imm) |
                ({32{b_type}} & b_imm);

assign inst_addi    = i_type & (func3 == 3'h0);
assign inst_sltiu   = i_type & (func3 == 3'h3);
assign inst_jalr    = (opcode == 7'h67);
assign inst_jal     = j_type;
assign inst_add     = r_type & (func3 == 3'h0) & (func7 == 7'h0);
assign inst_sub     = r_type & (func3 == 3'h0) & (func7 == 7'h20);
assign inst_lui     = (opcode == 7'h37);
assign inst_auipc   = (opcode == 7'h17);
assign inst_lw      = (opcode == 7'h3) & (func3 == 3'h2);
assign inst_lbu     = (opcode == 7'h3) & (func3 == 3'h4);
assign inst_sw      = s_type & (func3 == 3'h2);
assign inst_sb      = s_type & (func3 == 3'h0);
assign inst_beq     = b_type & (func3 == 3'h0);
assign inst_bne     = b_type & (func3 == 3'h1);
assign inst_ebreak  = (inst == 32'h100073); // no signal to i_type

/* ========= ouput =========== */
assign rs1_addr = inst[19:15]; 
assign rs2_addr = inst[24:20];
assign rd_addr = inst[11:7];
assign wen = (r_type | i_type | u_type | j_type) & (|rd_addr);

// add or addi
assign exu_op[0] = inst_add | inst_addi;
// lui or auipc
assign exu_op[1] = inst_lui | inst_auipc;
// sb or sw
assign exu_op[2] = s_type;
// jalr or jal
assign exu_op[3] = inst_jalr | inst_jal;
// ebreak
assign exu_op[4] = inst_ebreak;
// sub
assign exu_op[5] = inst_sub;
// unsigned or signed
assign exu_op[6] = inst_sltiu;
// sltiu
assign exu_op[7] = inst_sltiu;
// beq and bne
assign exu_op[9:8] = {inst_beq, inst_bne};

assign data1 = (r_type | i_type | s_type | b_type) ? src1 : (j_type ? pc : pc + 4);
assign data2 = (r_type | s_type | b_type) ? src2 : ((i_type | j_type) ? imm : 0);
assign data3 = (inst_jalr | inst_jal) ? pc + 4 : 0;
assign data4 = imm + { {32{inst_auipc}} & pc };

assign lsu_op = {inst_lw, inst_lbu, inst_sw, inst_sb};

assign wbu_op = {(r_type | i_type | u_type | j_type), (inst_lw | inst_lbu)};

endmodule
