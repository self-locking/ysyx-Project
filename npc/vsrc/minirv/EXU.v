module EXU (
    input wire [3:0] exu_op,
    input wire [31:0] data1,
    input wire [31:0] data2,
    input wire [31:0] data3,
    input wire [31:0] data4,
    output wire jump,
    output wire [31:0] jump_pc,
    output wire [31:0] exu_addr,
    output wire [31:0] exu_data
);

wire [31:0] add_o;

ADD add1 (
    .a      (data1      ),
    .b      (data2      ),
    .out    (add_o      )
);

assign jump = exu_op[3];
assign jump_pc = add_o;

// store or load
assign exu_addr = exu_op[2] ? data1 + data4 : add_o;
// lui, jalr, or add, addi, load
assign exu_data = exu_op[1] ? data4 : (exu_op[3] ? data3 : (exu_op[2] ? data2 : add_o));

endmodule