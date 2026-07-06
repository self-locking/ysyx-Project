import "DPI-C" function void halt(input int a);
module EXU (
    input wire clk,
    input wire pc,
    input wire [9:0] exu_op,
    input wire [31:0] data1,
    input wire [31:0] data2,
    input wire [31:0] data3,
    input wire [31:0] data4,
    input wire [31:0] a0,
    output wire jump,
    output wire [31:0] jump_pc,
    output wire [31:0] exu_addr,
    output wire [31:0] exu_data
);

wire [31:0] add_o;
wire lt;
wire equ;
reg [31:0] out;

always @(posedge clk) begin
    if (exu_op[4]) halt(a0);
end

ADD add1 (
    .a              (data1      ),
    .b              (data2      ),
    .is_sub         (exu_op[5]  ),
    .is_unsigned    (exu_op[6]  ),
    .out            (add_o      ),
    .lt             (lt         ),
    .equ            (equ        )
);

assign jump = exu_op[3] | (exu_op[8] & ~equ) | (exu_op[9] & equ);
assign jump_pc = (|exu_op[9:8]) ? pc + data4 : add_o;

// store or load
assign exu_addr = exu_op[2] ? data1 + data4 : add_o;

always @(*) begin
    if (exu_op[1])          out = data4;
    else if (exu_op[3])     out = data3;
    else if (exu_op[2])     out = data2;
    else if (exu_op[7])     out = { 31'b0, lt };
    else                    out = add_o;
end

// lui, jalr, or add, sltiu, addi, load
assign exu_data = out;
endmodule