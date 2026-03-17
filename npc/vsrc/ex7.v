module ex7(clk, resetn, ps2_clk, ps2_data, seg0, seg1, seg2, seg3, seg4, seg5);
	output [7:0] seg0, seg1, seg2, seg3, seg4, seg5;
	input clk, resetn;
	input ps2_clk, ps2_data;

	wire [7:0] w1, w2;
	wire valid;
	reg key_valid;
	reg [7:0] low_seg0, low_seg1, cen_seg0, cen_seg1, high_seg0, high_seg1;
	reg [7:0] count;
	reg stop;

	my_ps2_keyboard inst (
		.clk(clk),
		.resetn(resetn),
		.ps2_clk(ps2_clk),
		.ps2_data(ps2_data),
		.valid(valid),
		.out_data(w1),
		.stop(stop)
	);

	my_keyboardTrans inst0 (.data(w1), .asc(w2));

	my_seg inst1 (.sel(w1[3:0]), .seg(low_seg0));
	my_seg inst2 (.sel(w1[7:4]), .seg(low_seg1));

	my_seg inst3 (.sel(w2[3:0]), .seg(cen_seg0));
	my_seg inst4 (.sel(w2[7:4]), .seg(cen_seg1));

	my_seg inst5 (.sel(count[3:0]), .seg(high_seg0));
	my_seg inst6 (.sel(count[7:4]), .seg(high_seg1));

	always @(posedge stop or negedge resetn) begin
		if (!resetn) begin
			count <= 8'd0;
		end else begin
			count <= count + 1;
		end
	end

	always @(posedge clk or negedge resetn) begin
    if(!resetn) begin
        key_valid <= 0;
    end
    else begin
        if(valid) begin
            key_valid <= 1;
        end
        else if(stop) begin
            key_valid <= 0;
        end
    end
end

	assign seg0 = key_valid ? low_seg0 : 8'b11111111;
	assign seg1 = key_valid ? low_seg1 : 8'b11111111;
	assign seg2 = key_valid ? cen_seg0 : 8'b11111111;
	assign seg3 = key_valid ? cen_seg1 : 8'b11111111;

	assign seg4 = high_seg0;
	assign seg5 = high_seg1;

endmodule
