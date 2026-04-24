module LSU (
    input wire [3:0] lsu_op,
    input wire [31:0] exu_wdata,
    input wire [31:0] addr_i,
    input wire [31:0] rdata_i,
    output wire [31:0] addr_o1,
    output wire [31:0] addr_o2,
    output wire [31:0] rdata_o,
    output wire rdata_en,
    output wire wdata_en,
    output wire inst_sb,
    output wire [31:0] store_data
);

reg [31:0] sb_mask;
reg [31:0] lbu_data;
reg sel;
reg [1:0] lbu_sel;

always @(*) begin
    lbu_sel = addr_i[1:0];
    case (lbu_sel)
        2'b00: lbu_data = {24'h0, rdata_i[7:0]};
        2'b01: lbu_data = {24'h0, rdata_i[15:8]};
        2'b10: lbu_data = {24'h0, rdata_i[23:16]};
        2'b11: lbu_data = {24'h0, rdata_i[31:24]};
    endcase
end

assign addr_o1 = addr_i;
assign addr_o2 = addr_i;

assign rdata_o = lsu_op[3] ? rdata_i : (lsu_op[2] ? lbu_data : 32'h0);
assign rdata_en = lsu_op[2] | lsu_op[3];
assign wdata_en = lsu_op[1] | lsu_op[0];

always @(*) begin
    sel = lsu_op[1];
    case (sel)
        1'b0: sb_mask = 32'h000000FF;
        1'b1: sb_mask = 32'hFFFFFFFF;
    endcase 
end

assign inst_sb = lsu_op[0];
assign store_data =  exu_wdata & sb_mask;

endmodule

