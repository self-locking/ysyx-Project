module ROM(pc, inst);
	output [7:0] inst;
	input [3:0] pc;
	
	wire [7:0] com [15:0];

	assign com[0] = 8'b10001010; // li r0 1010
	assign com[1] = 8'b10010000; // li r1 0000
	assign com[2] = 8'b10100000; // li r2 0000
	assign com[3] = 8'b10110001; // li r3 0001 
	assign com[4] = 8'b00010111; // add r1 r1 r3 
	assign com[5] = 8'b00101001; // add r2 r2 r1
	assign com[6] = 8'b11010001; // benr0 r1 4 
	assign com[7] = 8'b01011110; // out r2 7
   	
	assign com[8] = 8'd0; // empty
	assign com[9] = 8'd0; 
	assign com[10] = 8'd0; 
	assign com[11] = 8'd0; 
	assign com[12] = 8'd0; 
	assign com[13] = 8'd0; 
	assign com[14] = 8'd0; 
	assign com[15] = 8'd0; 

	assign inst = com[pc];

endmodule
