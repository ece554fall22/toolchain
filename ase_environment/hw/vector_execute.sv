module vector_execute(
    input clk, rst_n,
    input [31:0] vdata1 [3:0],
    input [31:0] vdata2 [3:0],
    input [31:0] data1, data2,
    input [7:0] immediate,
    control_bus control,
    output [31:0] vdata_out [3:0],
    output [35:0] rdata_out
);

logic [31:0] reg_data;

vector_alu v_alu (.v1(vdata1), .v2(vdata2), .r1(data1), .r2(data2), .op(control.vector_alu_op), .imm(immediate), .clk(clk), .rst_n(rst_n), .en(1'b1), .vout(vdata_out), .rout(reg_data));

assign rdata_out = {4'b0, reg_data};

endmodule