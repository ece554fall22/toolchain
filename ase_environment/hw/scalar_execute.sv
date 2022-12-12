module scalar_execute(
    input clk, rst_n,
    input [35:0] data1, data2,
    control_bus control,
    input [24:0] immediate,
    output logic zero, sign, overflow,
    output logic [35:0] data_out
);

logic [35:0] operand2;

assign operand2 = (control.alu_operands) ? {{11{immediate[24]}}, immediate} : data2;

alu ALU (.A(data1), .B(operand2), .op(control.scalar_alu_op), .out(data_out), .zero(zero), .sign(sign), .overflow(overflow), .clk(clk), .rst_n(rst_n));

endmodule