module RAM #(ADDR_WIDTH = 32, DATA_WIDTH = 32) (
    input wire clk,
    input wire [ADDR_WIDTH-1:0] addr,
    input wire ren,
    input wire wen,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire [2:0] store,
    output wire [DATA_WIDTH-1:0] rdata
);

reg [3:0] mask;
reg [1:0] sel;
reg [DATA_WIDTH-1:0] pmem_rdata;

always @(*) begin
    sel = addr[1:0];
    if (store[0]) begin
        case (sel) 
            2'b00: mask = 4'b0001;
            2'b01: mask = 4'b0010;
            2'b10: mask = 4'b0100;
            2'b11: mask = 4'b1000;
        endcase
    end else if (store[1]) begin
        case (sel)
            2'b00: mask = 4'b0011;
            2'b10: mask = 4'b1100;
        endcase
    end else begin
        mask = 4'b1111;
    end
end

always @(posedge clk) begin
    if (wen) begin 
        pmem_write(addr >> 2, wdata, mask);
    end
end

always @(*) begin
    if (ren) begin
        pmem_rdata = pmem_read(addr >> 2, 1);
    end else begin
        pmem_rdata = 0;
    end
end

assign rdata = pmem_rdata;

endmodule