`define MEPC 32'h341
`define MSTATUS 32'h300
`define MCAUSE 32'h342
`define MTVEC 32'h305
`define MCYCLE 32'hb00
`define MCYCLEH 32'hb80
`define MVENDORID 32'hf11
`define MARCHID 32'hf12
module CSR (
    input wire clk,
    input wire rst,
    input wire [31:0] pc,
    input wire [31:0] addr,
    input wire [31:0] data,
    input wire wen,
    input wire [1:0] csr_op,
    output reg [31:0] csr
);

    reg [31:0] mepc, mstatus, mcause, mtvec, mcycle, mcycleh, mvendorid, marchid;

    always @(posedge clk) begin
        if (rst) begin
            mepc        <= 32'h0;
            mstatus     <= 32'h1800;
            mcause      <= 32'h0;
            mtvec       <= 32'h0;
            mcycle      <= 32'h0;
            mcycleh     <= 32'h0;
            mvendorid   <= 32'h79737978;
            marchid     <= 32'h18d3010;
        end else begin
            {mcycleh, mcycle} <= {mcycleh, mcycle} + 1'b1;

            if (wen) begin
                case (addr)
                    `MEPC:      mepc    <= data;
                    `MSTATUS:   mstatus <= data;
                    `MCAUSE:    mcause  <= data;
                    `MTVEC:     mtvec   <= data;
                    default:                   ;
                endcase
            end else if (csr_op[0]) begin
                mepc <= pc;
                mcause <= 32'hb;
            end
        end
    end

    always @(*) begin
        csr = 32'h0;
        if (csr_op[0]) begin
            csr = mtvec;
        end else if (csr_op[1]) begin
            csr = mepc;
        end else begin
            case (addr)
                    `MEPC:      csr = mepc;
                    `MSTATUS:   csr = mstatus;
                    `MCAUSE:    csr = mcause;
                    `MTVEC:     csr = mtvec;
                    `MCYCLE:    csr = mcycle;
                    `MCYCLEH:   csr = mcycleh;
                    `MVENDORID: csr = mvendorid;
                    `MARCHID:   csr = marchid;
                    default:    csr = 32'h0;
            endcase
        end
    end

endmodule
