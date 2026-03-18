module my_ps2_keyboard(
    input clk, resetn, ps2_clk, ps2_data,
    output reg [7:0] out_data,
    output reg valid,
	output reg stop
);

	reg [9:0] buffer;
	reg [3:0] count;
	reg [2:0] ps2_clk_sync;

	always @(posedge clk) begin
		ps2_clk_sync <= {ps2_clk_sync[1:0], ps2_clk}; // 下降采集
	end

	wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

	always @(posedge clk or negedge resetn) begin
		if (!resetn) begin
			count <= 0;
			out_data <= 8'd0;
			valid <= 1'b0;
			stop <= 1'b0;
		end else begin
			valid <= 1'b0; // 是否有信号

			if (sampling) begin
				if (count == 4'd10) begin
					if ((buffer[0] == 1'b0) && (ps2_data == 1'b1) && (^buffer[9:1])) begin
						reg [7:0] scancode;
						scancode = buffer[8:1];

						if (scancode == 8'hF0) begin
							stop <= 1'b1;  // 进入stop状态
						end else begin
							if (!stop) begin
								out_data <= scancode;
								valid <= 1'b1;
							end
							stop <= 1'b0;
						end
					end
					count <= 0;
				end else begin
					buffer[count] <= ps2_data;
					count <= count + 4'd1;
				end
			end
		end
	end

endmodule
