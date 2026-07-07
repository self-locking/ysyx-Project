module LSU (
    input wire [7:0] lsu_op,
    input wire [31:0] exu_data,
    input wire [31:0] addr,
    input wire [31:0] rdata_i,
    output wire [31:0] lsu_addr,
    output wire [31:0] rdata_o,
    output wire rdata_en,
    output wire wdata_en,
    output wire [31:0] store_data
);

reg [31:0] mask;
reg [31:0] lb_data;
reg [31:0] lh_data;
reg [2:0] sel;
reg [1:0] lb_sel;
reg [1:0] lh_sel;

always @(*) begin
    lb_sel = addr[1:0];
    case (lb_sel)
        2'b00: lb_data = { {24{lsu_op[7] & rdata_i[7]}}, rdata_i[7:0] };
        2'b01: lb_data = { {24{lsu_op[7] & rdata_i[15]}}, rdata_i[15:8] };
        2'b10: lb_data = { {24{lsu_op[7] & rdata_i[23]}}, rdata_i[23:16] };
        2'b11: lb_data = { {24{lsu_op[7] & rdata_i[31]}}, rdata_i[31:24] };
    endcase

    lh_sel = addr[1:0];
    case (lh_sel)
        2'b00: lh_data = { {16{lsu_op[6] & rdata_i[15]}}, rdata_i[15:0] };
        2'b10: lh_data = { {16{lsu_op[6] & rdata_i[31]}}, rdata_i[31:16] };
        default: lh_data = 32'h0;
    endcase
end

assign lsu_addr = addr;

assign rdata_o = lsu_op[5] ? rdata_i : 
                ((lsu_op[7] | lsu_op[4]) ? lb_data : 
                ((lsu_op[6] | lsu_op[3]) ? lh_data : 32'h0));
assign rdata_en = |lsu_op[7:3];
assign wdata_en = |lsu_op[2:0];

always @(*) begin
    sel = lsu_op[2:0];
    case (sel)
        3'b001: mask = 32'h000000FF;
        3'b010: mask = 32'h0000FFFF;
        default: mask = 32'hFFFFFFFF;
    endcase 
end

assign store_data = exu_data & mask;

endmodule

