module RAM(clk, rst, en, wdata, waddr, addr1, addr2, data1, data2);
	output [7:0] data1, data2;
	input clk, en, rst;
	input [7:0] wdata;
	input [1:0] waddr, addr1, addr2;

	reg [7:0] r [3:0];

	always@(posedge clk) begin
		if (rst) begin
			r[0] <= 0;
			r[1] <= 0;
			r[2] <= 0;
			r[3] <= 0;
		end else begin
			if (en) begin
				r[waddr] <= wdata;
			end
		end
	end

	assign data1 = r[addr1];
	assign data2 = r[addr2];

endmodule
