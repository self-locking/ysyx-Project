module EXU (
    input wire clk,
    input wire [31:0] pc,
    input wire [17:0] exu_op,
    input wire [31:0] data1,
    input wire [31:0] data2,
    input wire [31:0] data3,
    input wire [31:0] data4,
    input wire [31:0] a0,
    output wire jump,
    output wire [31:0] jump_pc,
    output wire [31:0] exu_addr,
    output wire [31:0] exu_data
);

wire [31:0] add_res;
wire [31:0] slt_res;
wire [31:0] xor_res;
wire [31:0] or_res;
wire [31:0] sra_res;
wire [31:0] sll_res;
wire [31:0] and_res;
wire [31:0] srl_res;
wire lt;
wire equ;

always @(posedge clk) begin
    if (exu_op[4]) halt(a0);
end

ADD add1 (
    .a              (data1      ),
    .b              (data2      ),
    .is_sub         (exu_op[5]  ),
    .is_unsigned    (exu_op[6]  ),
    .out            (add_res    ),
    .lt             (lt         ),
    .equ            (equ        )
);

assign jump = exu_op[3] | (exu_op[11] & equ) | (exu_op[10] & ~equ) | (exu_op[9] & lt) | (exu_op[8] & ~lt);
assign jump_pc = (|exu_op[11:8]) ? pc + data4 : add_res;

// store or load
assign exu_addr = exu_op[2] ? data1 + data4 : add_res;

assign slt_res = {31'b0, lt};
assign xor_res = data1 ^ data2;
assign or_res = data1 | data2;
assign sra_res = $signed(data1) >>> data2[4:0];
assign sll_res = data1 << data2[4:0];
assign and_res = data1 & data2;
assign srl_res = data1 >> data2[4:0];

assign exu_data = {32{(exu_op[0] | exu_op[5])}} & add_res |
                  {32{exu_op[1]}} & data4 | 
                  {32{exu_op[3]}} & data3 | 
                  {32{exu_op[2]}} & data2 | 
                  {32{exu_op[7]}} & slt_res | 
                  {32{exu_op[12]}} & xor_res | 
                  {32{exu_op[13]}} & or_res | 
                  {32{exu_op[14]}} & sra_res | 
                  {32{exu_op[15]}} & sll_res |
                  {32{exu_op[16]}} & and_res |
                  {32{exu_op[17]}} & srl_res;

endmodule