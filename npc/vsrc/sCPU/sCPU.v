module sCPU(clk, rst, en, seg0, seg1);
	output [7:0] seg0, seg1;
	input clk, rst, en;

	wire pc_en, ram_en, li, add, benr0, out, skip;
	wire [1:0] addr1, addr2, waddr;
	wire [7:0] wdata, inst, data1, data2, sum, SEG0, SEG1;
	wire [3:0] pc, pc_data, imm, adata;
	
	assign li = inst[7:6] == 2'b10;
	assign add = inst[7:6] == 2'b00;
	assign benr0 = inst[7:6] == 2'b11;
	assign out = inst[7:6] == 2'b01;
	
	assign addr1 = (benr0) ? 2'b00 : inst[3:2];
	assign addr2 = inst[1:0];
	assign pc_en = en && (benr0 || out) && skip;
	assign ram_en = en && (li || add);
	assign imm = inst[3:0];
	assign sum = data1 + data2;

	assign wdata = li ? {4'b0000, imm} : (add ? sum : 8'd0);
	assign waddr = inst[5:4];
	assign skip = benr0 ? (data1 != data2) : out;
	assign pc_data = inst[5:2];

	PC my_pc (
		.clk(clk),
		.rst(rst),
		.en(pc_en),
		.data_in(pc_data),
		.data_out(pc)
	);
	
	ROM my_rom (
		.pc(pc),
		.inst(inst)
	);

	RAM my_ram (
		.clk(clk),
		.rst(rst),
		.en(ram_en),
		.wdata(wdata),
		.waddr(waddr),
		.addr1(addr1),
		.addr2(addr2),
		.data1(data1),
		.data2(data2)
	);

	seg my_seg0 (.sel(data2[3:0]), .seg(SEG0));
	seg my_seg1 (.sel(data2[7:4]), .seg(SEG1));
	assign seg0 = out ? SEG0 : 8'b00000000;
	assign seg1 = out ? SEG1 : 8'b00000000;

endmodule
