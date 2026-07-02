`ifdef ITRACE
import "DPI-C" function void disassemble_and_log(input int pc, input int insn);
`endif
module IFU (
    input wire clk,
    input wire rst,
    input wire jump,
    input wire [31:0] jump_pc,
    output wire [31:0] pc_o,
    output wire [31:0] inst,
    output wire [31:0] snpc
);

reg [31:0] pc;

always @(posedge clk) begin
    if (rst) pc <= 32'h80000000;
    else begin
        pc <= (jump) ? jump_pc : pc + 4;
        `ifdef ITRACE
            disassemble_and_log(pc, inst);
        `endif
    end
end

assign pc_o = pc;
assign inst = pmem_read(pc >> 2, 0);
assign snpc = pc + 4;

endmodule
