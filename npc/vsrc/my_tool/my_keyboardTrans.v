module my_keyboardTrans(
    input  [7:0] data,
    output reg [7:0] asc
);

always @(*) begin
    case(data)
        8'h45: asc = "0";
        8'h16: asc = "1";
        8'h1E: asc = "2";
        8'h26: asc = "3";
        8'h25: asc = "4";
        8'h2E: asc = "5";
        8'h36: asc = "6";
        8'h3D: asc = "7";
        8'h3E: asc = "8";
        8'h46: asc = "9";
        
		8'h1C: asc = "a";
        8'h32: asc = "b";
        8'h21: asc = "c";
        8'h23: asc = "d";
        8'h24: asc = "e";
        8'h2B: asc = "f";
        8'h34: asc = "g";
        8'h33: asc = "h";
        8'h43: asc = "i";
        8'h3B: asc = "j";
        8'h42: asc = "k";
        8'h4B: asc = "l";
        8'h3A: asc = "m";
        8'h31: asc = "n";
        8'h44: asc = "o";
        8'h4D: asc = "p";
        8'h15: asc = "q";
        8'h2D: asc = "r";
        8'h1B: asc = "s";
        8'h2C: asc = "t";
        8'h3C: asc = "u";
        8'h2A: asc = "v";
        8'h1D: asc = "w";
        8'h22: asc = "x";
        8'h35: asc = "y";
        8'h1A: asc = "z";

        default: asc = 8'd0;
    endcase
end

endmodule
