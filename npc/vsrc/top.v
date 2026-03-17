// 顶层文件名为top，因此等会include的头文件为Vtop
// 若顶层文件名为example，则include的头文件名为Vexample

module top(
	input clk,
	input rst,
	output reg[15:0] f
);

	reg[31:0] ct;	

	always @(posedge clk or posedge rst) begin
		if (rst) begin
			ct <= 0;
			f <= 16'd1;
		end else begin
			if (ct == 100_000_00 - 1) begin
				ct <= 0;
				f <= {f[14:0], f[15]};
			end else begin
				ct <= ct + 1;
			end
		end
	end
endmodule

// module top(
// 	input a,
// 	input b,
// 	input rst,
// 	output f
// );

// 	assign f = rst ? 0 : a ^ b;

// endmodule
