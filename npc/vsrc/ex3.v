module ex3(
	output overflow, carry, zero,
	output [7:0] hex0, hex1, hex2, hex3,
	input [3:0] A, B,
	input [2:0] sel
);

	reg [3:0] result;
	reg cout, sig;
	
	always @(*) begin
		result = 4'd0;
		cout = 0;
		sig = 0;
		case(sel)
			3'b000: begin
				{cout, result} = A + B; 
				sig = 1;
			end
			3'b001: begin
				{cout, result} = A - B;
				sig = 1;
			end
			3'b010: result = ~A;
			3'b011: result = A & B;
			3'b100: result = A | B;
			3'b101: result = A ^ B;
			3'b110: begin
				if(A[3] & ~B[3]) begin
					result = 1;
				end else if (~A[3] & B[3]) begin
					result = 0;
				end else if (~A[3] & ~B[3]) begin
					result = (A[2:0] < B[2:0]) ? 1 : 0;
				end else begin
					result = (~A > ~B) ? 4'd1 : 4'd0;
				end
			end
			3'b111: result = (A == B) ? 4'd1 : 4'd0;
		endcase	
	end

	my_seg inst0 (.sel({3'b000, result[0]}), .seg(hex0));
	my_seg inst1 (.sel({3'b000, result[1]}), .seg(hex1));
	my_seg inst2 (.sel({3'b000, result[2]}), .seg(hex2));
	my_seg inst3 (.sel({3'b000, result[3]}), .seg(hex3));

	assign zero = ~(|result);
	assign carry = cout;
	assign overflow = (sig) ? (A[3] & B[3] & ~result[3]) || (~A[3] & ~B[3] & result[3]) : 0;

endmodule
