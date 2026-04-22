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

reg [DATA_WIDTH-1:0] mem [31:0];

assign mem[0] = 32'h02000093;       // addi x1, x0, 32
assign mem[1] = 32'h12345137;       // lui x2, 74565
assign mem[2] = 32'h67810113;       // addi x2, x2, 1656
assign mem[3] = 32'h0020A023;       // sw x2, 0(x1)
assign mem[4] = 32'hFFF00193;       // addi x3, x0, -1
assign mem[5] = 32'h003080A3;       // sb x3, 1(x1)
assign mem[6] = 32'h0;              //
assign mem[7] = 32'h00c48633;       //
assign mem[8] = 32'hfffff7b7;       //

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
        if (inst_sb) mem[byte_waddr] <= ((mem[byte_waddr] & ~mask) | repeat_data & mask);
        else mem[byte_waddr] <= wdata;
    end
end

assign byte_raddr = raddr[5:2];
assign rdata = mem[byte_raddr];

assign byte_pc = pc[5:2];
assign inst = mem[byte_pc];

endmodule