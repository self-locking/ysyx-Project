module IFU (
    input wire clk,
    input wire rst,
    input wire [31:0] inst_i,
    input wire jump,
    input wire [31:0] jump_pc,
    output wire [31:0] pc_o,
    output wire [31:0] inst_o,
    output wire [31:0] snpc
);

reg [31:0] pc;

always @(posedge clk) begin
    if (rst) pc <= 32'h80000000;
    else pc <= (jump) ? jump_pc : pc + 4;
end

assign pc_o = pc;
assign inst_o = inst_i;
assign snpc = pc + 4;

endmodule
