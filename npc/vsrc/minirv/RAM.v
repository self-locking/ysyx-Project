module RAM #(ADDR_WIDTH = 32, DATA_WIDTH = 32) (
    input wire clk,
    input wire [ADDR_WIDTH-1:0] raddr,
    input wire ren,
    input wire [ADDR_WIDTH-1:0] waddr,
    input wire wen,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire inst_sb,
    output wire [DATA_WIDTH-1:0] rdata
);

reg [3:0] mask;
reg [1:0] sel;
reg [DATA_WIDTH-1:0] pmem_rdata;

always @(*) begin
    sel = waddr[1:0];
    if (inst_sb) begin
        case (sel) 
            2'b00: mask = 4'b0001;
            2'b01: mask = 4'b0010;
            2'b10: mask = 4'b0100;
            2'b11: mask = 4'b1000;
        endcase
    end else begin
        mask = 4'b0;
    end
end

always @(posedge clk) begin
    if (wen) begin 
        pmem_write(waddr >> 2, wdata, mask);
    end
end

always @(*) begin
    if (ren) begin
        pmem_rdata = pmem_read(raddr >> 2, 1);
    end else begin
        pmem_rdata = 0;
    end
end

assign rdata = pmem_rdata;

endmodule