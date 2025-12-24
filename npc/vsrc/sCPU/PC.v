module PC(clk, rst, en, data_in, data_out);
	output reg [3:0] data_out;
	input [3:0] data_in;
	input clk, rst, en;

	always@(posedge clk) begin
		if (rst) begin
			data_out <= 0;
		end else begin
			if (!en) begin
				data_out <= data_out + 1;
			end else begin
				data_out <= data_in;
			end
		end
	end

endmodule
		
