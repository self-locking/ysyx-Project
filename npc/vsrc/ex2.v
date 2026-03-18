module ex2(
	output reg [2:0] out,
	output reg signal,
	output reg [7:0] hex0,
	input [7:0] sw,
	input en
);

	always @(*) begin
		if(!en) begin
			signal = 0;
			out = 0;
			hex0 = 0;
		end else begin	
			casez(sw)
				8'b1zzzzzzz: begin
					signal = 1;
					out = 3'b111;
					hex0 = ~8'b11100000;
				end
				8'b01zzzzzz: begin
					signal = 1;
					out = 3'b110;
					hex0 = ~8'b10111110;
				end
				8'b001zzzzz: begin
					signal = 1;
					out = 3'b101;
					hex0 = ~8'b10110110;
				end
				8'b0001zzzz: begin
					signal = 1;
					out = 3'b100;
					hex0 = ~8'b01100110;
				end
				8'b00001zzz: begin
					signal = 1;
					out = 3'b011;
					hex0 = ~8'b11110010;
				end
				8'b000001zz: begin
					signal = 1;
					out = 3'b010;
					hex0 = ~8'b11011010;
				end
				8'b0000001z: begin
					signal = 1;
					out = 3'b001;
					hex0 = ~8'b01100000;
				end
				8'b00000001: begin
					signal = 1;
					out = 3'b000;
					hex0 = ~8'b00000000;
				end
				default: begin
					signal = 0;
					out = 3'b000;
					hex0 = ~8'b00000000;
				end
			endcase
		end
	end
endmodule
		
