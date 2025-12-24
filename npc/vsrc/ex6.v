module ex6(in, clk, reset, hex0, hex1);
	output [7:0] hex0, hex1;
	input [7:0] in;
	input clk, reset;

	reg [7:0] q;
	reg sig;
	
	always @(posedge clk or posedge reset) begin
		if (reset) begin
			q <= in;
			sig <= |in;
		end else begin
			q <= (sig) ? {(q[0]^q[2]^q[3]^q[4]), q[7:1]} : 8'd0;
		end
	end

	my_seg inst0 (.sel(q[3:0]), .seg(hex0));
	my_seg inst1 (.sel(q[7:4]), .seg(hex1));

endmodule
