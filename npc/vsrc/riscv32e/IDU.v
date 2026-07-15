module IDU (
    input wire [31:0] src1,
    input wire [31:0] src2,
    input wire [31:0] inst,
    input wire [31:0] pc,
    output wire [4:0] rs1_addr,
    output wire [4:0] rs2_addr,
    output wire [4:0] rd_addr,
    output wire wen,
    output wire [21:0] exu_op, // can add more bits
    output wire [31:0] data1,
    output wire [31:0] data2,
    output wire [31:0] data3,
    output wire [31:0] data4,
    output wire [7:0] lsu_op,
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

wire inst_add_addi;
wire inst_sltu_sltiu;
wire inst_sra_srai;
wire inst_sll_slli;
wire inst_srl_srli;
wire inst_jalr;
wire inst_sub;
wire inst_xor_xori;
wire inst_or_ori;
wire inst_and_andi;
wire inst_lui;
wire inst_auipc;
wire inst_lb;
wire inst_lh;
wire inst_lw;
wire inst_lbu;
wire inst_lhu;
wire inst_sw;
wire inst_sh;
wire inst_sb;
wire inst_beq;
wire inst_bne;
wire inst_blt;
wire inst_bge;
wire inst_bltu;
wire inst_bgeu;
wire inst_ebreak;
wire inst_csrrw;
wire inst_csrrs;
wire inst_ecall;
wire inst_mret;

/* ========= ready =========== */
assign opcode = inst[6:0];
assign func3  = inst[14:12];
assign func7  = inst[31:25];

assign r_type = (opcode == 7'h33);
assign i_type = (opcode == 7'h13) | (opcode == 7'h3) | (opcode == 7'h67) | (opcode == 7'h73);
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

assign inst_add_addi    =   ~inst_jalr &
                            ( (i_type & (func3 == 3'h0)) | 
                            (r_type & (func3 == 3'h0) & (func7 == 7'h0)) );
assign inst_sltu_sltiu  =   (i_type & (func3 == 3'h3)) |
                            (r_type & (func3 == 3'h3) & (func7 == 7'h0));
assign inst_sra_srai    =   (i_type & (func3 == 3'h5) & (func7 == 7'h20)) |
                            (r_type & (func3 == 3'h5) & (func7 == 7'h20));
assign inst_sll_slli    =   (i_type & (func3 == 3'h1) & (func7 == 7'h0)) |
                            (r_type & (func3 == 3'h1) & (func7 == 7'h0));
assign inst_srl_srli    =   (i_type & (func3 == 3'h5) & (func7 == 7'h0)) |
                            (r_type & (func3 == 3'h5) & (func7 == 7'h0));
assign inst_jalr    = (opcode == 7'h67);

assign inst_jal     = j_type;

assign inst_sub     = r_type & (func3 == 3'h0) & (func7 == 7'h20);
assign inst_xor_xori    =   (i_type & (func3 == 3'h4)) |
                            (r_type & (func3 == 3'h4) & (func7 == 7'h0));
assign inst_or_ori      =   (i_type & (func3 == 3'h6)) |
                            (r_type & (func3 == 3'h6) & (func7 == 7'h0));

assign inst_and_andi    =   (i_type & (func3 == 3'h7)) |
                            (r_type & (func3 == 3'h7) & (func7 == 7'h0));
assign inst_lui     = (opcode == 7'h37);
assign inst_auipc   = (opcode == 7'h17);

assign inst_lb      = (opcode == 7'h3) & (func3 == 3'h0);
assign inst_lh      = (opcode == 7'h3) & (func3 == 3'h1);
assign inst_lw      = (opcode == 7'h3) & (func3 == 3'h2);
assign inst_lbu     = (opcode == 7'h3) & (func3 == 3'h4);
assign inst_lhu     = (opcode == 7'h3) & (func3 == 3'h5);

assign inst_sw      = s_type & (func3 == 3'h2);
assign inst_sh      = s_type & (func3 == 3'h1);
assign inst_sb      = s_type & (func3 == 3'h0);

assign inst_beq     = b_type & (func3 == 3'h0);
assign inst_bne     = b_type & (func3 == 3'h1);
assign inst_blt     = b_type & (func3 == 3'h4);
assign inst_bge     = b_type & (func3 == 3'h5);
assign inst_bltu    = b_type & (func3 == 3'h6);
assign inst_bgeu    = b_type & (func3 == 3'h7);

assign inst_csrrw   = i_type & (func3 == 3'h1);
assign inst_csrrs   = i_type & (func3 == 3'h2);

assign inst_ebreak  = (inst == 32'h100073);
assign inst_ecall   = (inst == 32'h73);
assign inst_mret  = (inst == 32'h30200073);

/* ========= ouput =========== */
assign rs1_addr = inst[19:15]; 
assign rs2_addr = inst[24:20];
assign rd_addr = inst[11:7];
assign wen = (r_type | i_type | u_type | j_type) & (|rd_addr);

// add or addi
assign exu_op[0] = inst_add_addi;
// lui or auipc
assign exu_op[1] = inst_lui | inst_auipc;
// sb, sh, sw
assign exu_op[2] = s_type;
// jalr or jal
assign exu_op[3] = inst_jalr | inst_jal;
// ebreak
assign exu_op[4] = inst_ebreak;
// sub
assign exu_op[5] = inst_sub;
// unsigned or signed
assign exu_op[6] = inst_sltu_sltiu | inst_bltu | inst_bgeu;
// sltiu, sltu
assign exu_op[7] = inst_sltu_sltiu;
// beq and bne
assign exu_op[11:8] = {inst_beq, inst_bne, (inst_blt | inst_bltu), (inst_bge | inst_bgeu)};
// xor, xori
assign exu_op[12] = inst_xor_xori;
// or, ori
assign exu_op[13] = inst_or_ori;
// sra, srai
assign exu_op[14] = inst_sra_srai;
// sll, slli
assign exu_op[15] = inst_sll_slli;
// and, andi
assign exu_op[16] = inst_and_andi;
// srl, srli
assign exu_op[17] = inst_srl_srli;
// csrrw, csrrs
assign exu_op[19:18] = {inst_csrrs, inst_csrrw};
// ecall
assign exu_op[20] = inst_ecall;
// mret
assign exu_op[21] = inst_mret;

assign data1 = (r_type | i_type | s_type | b_type) ? src1 : (j_type ? pc : pc + 4);
assign data2 = (r_type | s_type | b_type) ? src2 : ((i_type | j_type) ? imm : 0);
assign data3 = (inst_jalr | inst_jal) ? pc + 4 : 0;
assign data4 = imm + { {32{inst_auipc}} & pc };

assign lsu_op = {inst_lb, inst_lh, inst_lw, inst_lbu, inst_lhu, inst_sw, inst_sh, inst_sb};

assign wbu_op = {(r_type | i_type | u_type | j_type), (inst_lw | inst_lbu | inst_lh | inst_lhu | inst_lb)};

endmodule
