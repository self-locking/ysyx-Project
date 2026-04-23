import "DPI-C" function int pmem_read(input int raddr);
import "DPI-C" function void pmem_write(input int waddr, input int wdata);
module RAM #(ADDR_WIDTH = 32, DATA_WIDTH = 32) (
    input wire clk,
    input wire [ADDR_WIDTH-1:0] pc,
    input wire [ADDR_WIDTH-1:0] raddr,
    input wire [ADDR_WIDTH-1:0] waddr,
    input wire wen,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire inst_sb,
    output wire [DATA_WIDTH-1:0] rdata,
    output wire [DATA_WIDTH-1:0] inst
);

wire [3:0] byte_waddr;
wire [3:0] byte_raddr;
wire [1:0] BE;
reg [31:0] mask;
reg [1:0] sel;
wire [3:0] byte_pc;
wire [31:0] repeat_data;

assign byte_waddr = waddr[5:2];
assign BE = waddr[1:0];
assign repeat_data = {4{wdata[7:0]}};

always @(*) begin
    sel = BE;
    case (sel) 
        2'b00: mask = 32'h000000FF;
        2'b01: mask = 32'h0000FF00;
        2'b10: mask = 32'h00FF0000;
        2'b11: mask = 32'hFF000000;
    endcase
end

always @(posedge clk) begin
    if (wen) begin 
        if (inst_sb) pmem_write(byte_waddr, (pmem_read(byte_waddr) & ~mask) | (repeat_data & mask) );
        else pmem_write(byte_waddr, wdata);
    end
end

assign byte_raddr = raddr[5:2];
assign rdata = pmem_read(byte_raddr);

assign byte_pc = pc[5:2];
assign inst = pmem_read(byte_pc);

endmodule