module GPR #(ADDR_WIDTH = 5, DATA_WIDTH = 32) (
    input wire clk,
    input wire rst,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire [ADDR_WIDTH-1:0] waddr,
    input wire wen,
    input wire [ADDR_WIDTH-1:0] addr1,
    input wire [ADDR_WIDTH-1:0] addr2,
    output wire [DATA_WIDTH-1:0] src1,
    output wire [DATA_WIDTH-1:0] src2,
    output wire [DATA_WIDTH-1:0] a0
);

reg [DATA_WIDTH-1:0] rf [2**(ADDR_WIDTH-1)-1:0];

integer i;

always @(posedge clk) begin
    if (rst) begin
        for (i = 1; i < 2**ADDR_WIDTH; i = i + 1) begin
            rf[i] <= {DATA_WIDTH{1'b0}};
        end
    end
    else if (wen) begin
        rf[waddr] <= wdata;
    end 
end

assign src1 = rf[addr1];
assign src2 = rf[addr2];
assign a0 = rf[10];


endmodule