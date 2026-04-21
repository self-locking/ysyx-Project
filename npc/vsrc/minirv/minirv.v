module minirv (
    input wire clk,
    input wire rst,
    output wire result
);

/*============ IFU ==============*/
wire [31:0] pc;
wire [31:0] snpc;
wire [31:0] inst;
wire jump;
wire [31:0] jump_pc;

/*============ IDU ==============*/
wire [31:0] data1;
wire [31:0] data2;
wire [31:0] data3;
wire [31:0] data4;
wire [2:0] exu_op; // can add more bits
wire [31:0] rd_addr;

/*============ EXU ==============*/
wire [31:0] exu_addr;
wire [31:0] exu_data;

/*============ LSU ==============*/
wire [3:0] lsu_op;
wire [31:0] lsu_rdata;
wire [31:0] lsu_wdata;

/*============ RAM ==============*/
wire ram_wen;
wire [31:0] ram_rdata;
wire [31:0] ram_inst;

/*============ WBU ==============*/
wire [1:0] wbu_op;
wire [31:0] wbu_wdata;

/*============ GPR ==============*/
wire reg_wen;
wire [4:0] rs1_addr;
wire [4:0] rs2_addr;
wire [31:0] src1;
wire [31:0] src2;

IFU ifu1 (
    .clk        (clk        ),
    .rst        (rst        ),
    .inst_i     (ram_inst   ),
    .pc_o       (pc         ),
    .inst_o     (inst       ),    
    .jump       (jump       ),
    .jump_pc    (jump_pc    ),
    .snpc       (snpc       )
);

IDU idu2 (
    .src1       (src1       ),
    .src2       (src2       ),
    .inst_i     (inst       ),
    .pc         (pc         ),
    .snpc       (snpc       ),
    .rs1_addr   (rs1_addr   ),
    .rs2_addr   (rs2_addr   ),
    .rd_addr    (rd_addr    ),
    .wen        (reg_wen    ),
    .exu_op     (exu_op     ),
    .data1      (data1      ),
    .data2      (data2      ),
    .data3      (data3      ),
    .imm        (imm        ),
    .lsu_op     (lsu_op     ),
    .wbu_op     (wbu_op     )
);

EXU exu3 (
    .exu_op     (exu_op     ),
    .data1      (data1      ),
    .data2      (data2      ),
    .data3      (data3      ),
    .imm        (imm        ),
    .jump       (jump       ),
    .jump_pc    (jump_pc    ),
    .exu_addr   (exu_addr   ),
    .exu_data   (exu_data   )
);

LSU lsu4 (
    .lsu_op     (lsu_op     ),
    .exu_wdata  (exu_data   ),
    .addr       (exu_addr   ),
    .rdata_i    (ram_rdata  ),
    .rdata_o    (lsu_rdata  ),
    .wdata_en   (ram_wen    ),
    .wdata      (lsu_wdata  )
);

RAM ram5 (
    .clk        (clk        ),
    .pc         (pc         ),
    .en         (ram_wen    ),
    .wdata      (lsu_wdata  ),
    .rdata      (ram_rdata  ),
    .inst       (ram_inst   ),
);

WBU wbu6 (
    .lsu_data   (lsu_rdata  ),
    .exu_data   (exu_data   ),
    .wbu_op     (wbu_op     ),
    .reg_wen    (reg_wen   ),
    .wbu_data   (wbu_data   )
);

GPR gpr7 (
    .clk        (clk        ),
    .wdata      (wbu_data   ),
    .waddr      (rd_addr    ),
    .wen        (reg_wen    ),
    .addr1      (rs1_addr   ),
    .addr2      (rs2_addr   ),
    .src1       (src1       ),
    .src2       (src2       )
);

endmodule

