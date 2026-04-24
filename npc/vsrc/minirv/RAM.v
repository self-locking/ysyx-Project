import "DPI-C" function int pmem_read(input int raddr);
import "DPI-C" function void pmem_write(input int waddr, input int wdata, input int mask);
module RAM #(ADDR_WIDTH = 32, DATA_WIDTH = 32) (
    input wire clk,
    input wire [ADDR_WIDTH-1:0] pc,
    input wire [ADDR_WIDTH-1:0] raddr,
    input wire ren,
    input wire [ADDR_WIDTH-1:0] waddr,
    input wire wen,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire inst_sb,
    output wire [DATA_WIDTH-1:0] rdata,
    output wire [DATA_WIDTH-1:0] inst
);

reg [2:0] mask;
reg [1:0] sel;
reg [DATA_WIDTH-1:0] pmem_rdata;

always @(*) begin
    sel = waddr[1:0];
    if (inst_sb) begin
        case (sel) 
            2'b00: mask = 3'd1;
            2'b01: mask = 3'd2;
            2'b10: mask = 3'd3;
            2'b11: mask = 3'd4;
        endcase
    end else begin
        mask = 0;
    end
end

always @(posedge clk) begin
    if (wen) begin 
        pmem_write(waddr >> 2, wdata, mask);
    end
end

always @(*) begin
    if (ren) begin
        pmem_rdata = pmem_read(raddr >> 2);
    end else begin
        pmem_rdata = 0;
    end
end

assign rdata = pmem_rdata;
assign inst = pmem_read(pc >> 2);

endmodule