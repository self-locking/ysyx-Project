module WBU (
    input wire [31:0] lsu_data,
    input wire [31:0] exu_data,
    input wire [1:0] wbu_op,
    output wire [31:0] wbu_data
);

assign wbu_data = wbu_op[0] ? lsu_data : exu_data;

endmodule