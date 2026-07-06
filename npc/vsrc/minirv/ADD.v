module ADD(
    input  wire [31:0] a,
    input  wire [31:0] b,
    input  wire        is_sub,
    input  wire        is_unsigned,
    output wire [31:0] out,
    output wire        lt,
    output wire        equ
    // output wire        ge
);

    wire    [31:0]  complement;

    assign complement = b ^ {32{is_sub}};
    assign out = a + complement + is_sub;

    assign lt = is_unsigned ? (a < b) : ($signed(a) < $signed(b));

    assign equ = a == b;
    // assign ge  = ~lt;
endmodule
