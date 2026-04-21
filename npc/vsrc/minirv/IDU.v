module IDU (
    input wire [31:0] src1,
    input wire [31:0] src2,
    input wire [31:0] inst_i,
    input wire [31:0] pc,
    input wire [31:0] snpc,
    output wire [4:0] rs1_addr,
    output wire [4:0] rs2_addr,
    output wire [4:0] rd_addr,
    output wire wen,
    output wire [2:0] exu_op, // can add more bits
    output wire [31:0] data1,
    output wire [31:0] data2,
    output wire [31:0] data3,
    output wire [31:0] data4,
    output wire [3:0] lsu_op,
    output wire [1:0] wbu_op
);

wire [6:0] opcode;
wire [2:0] func;

wire r_type;
wire i_type;
wire s_type;
wire u_type;

wire [31:0] imm;
wire [31:0] i_imm;
wire [31:0] s_imm;
wire [31:0] u_imm;

wire inst_addi;
wire inst_jalr;
wire inst_add;
wire inst_lui;
wire inst_lw;
wire inst_lbu;
wire inst_sw;
wire inst_sb;

/* ========= ready =========== */
assign opcode = inst_i[6:0];
assign func   = inst_i[14:12];

assign r_type = (opcode == 7'h33);
assign i_type = (opcode == 7'h13) | (opcode == 7'h3) | (opcode == 7'h67);
assign s_type = (opcode == 7'h23);
assign u_type = (opcode == 7'h37);

assign i_imm = { {20{inst_i[31]}}, inst_i[31:20] };
assign s_imm = { {20{inst_i[31]}}, inst_i[31:25], inst_i[11:7] };
assign u_imm = { inst_i[31:12], {12{1'b0}} };
assign imm = ({32{i_type}} & i_imm) | ({32{s_type}} & s_imm) |({32{u_type}} & u_imm);

assign inst_addi = i_type & (func == 3'b0);
assign inst_jalr = (opcode == 7'h67);
assign inst_add = r_type & (func == 3'b0);
assign inst_lui = (opcode == 7'h37);
assign inst_lw = (opcode == 7'h3) & (func == 3'h2);
assign inst_lbu = (opcode == 7'h3) & (func == 3'h4);
assign inst_sw = s_type & (func == 3'h2);
assign inst_sb = s_type & (func == 3'h0);

/* ========= ouput =========== */
assign rs1_addr = inst_i[19:15]; 
assign rs2_addr = inst_i[24:20];
assign rd_addr = inst_i[11:7];
assign wen = r_type | i_type | u_type;

// add or addi
assign exu_op[0] = inst_add | inst_addi;
// lui
assign exu_op[1] = inst_lui;
// sb or sw
assign exu_op[2] = s_type;

assign data1 = (r_type | i_type | s_type) ? src1 : snpc;
assign data2 = (r_type | s_type) ? src2 : ((i_type) ? imm : 0);
assign data3 = inst_jalr ? snpc : 0;
assign data4 = imm;

assign lsu_op = {inst_lw, inst_lbu, inst_sw, inst_sb};

assign wbu_op = {(r_type | i_type | u_type), (inst_lw | inst_lbu)};

endmodule
