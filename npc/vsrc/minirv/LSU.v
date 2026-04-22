module LSU (
    input wire [3:0] lsu_op,
    input wire [31:0] exu_wdata,
    input wire [31:0] addr_i,
    input wire [31:0] rdata_i,
    output wire [31:0] addr_o1,
    output wire [31:0] addr_o2,
    output wire [31:0] rdata_o,
    output wire wdata_en,
    output wire inst_sb,
    output wire [31:0] store_data
);

reg [31:0] mask;
reg sel;

assign addr_o1 = addr_i;
assign addr_o2 = addr_i;

assign rdata_o = lsu_op[3] ? rdata_i : (lsu_op[2] ? { {24{1'b0}}, rdata_i[7:0] } : 32'h0);
assign wdata_en = lsu_op[1] | lsu_op[0];

always @(*) begin
    sel = lsu_op[1];
    case (sel)
        1'b0: mask = 32'h000000FF;
        1'b1: mask = 32'hFFFFFFFF;
    endcase 
end

assign inst_sb = lsu_op[0];
assign store_data =  exu_wdata & mask;

endmodule

