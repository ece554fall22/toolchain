module fp_alu(A, B, op, out, gt);

input [31:0] A, B;
input [1:0] op;
output logic [31:0] out;
output logic gt;

shortreal operand1, operand2, flout_out;

assign out = $shortrealtobits(flout_out);

always_comb begin
     operand1 = $bitstoshortreal(A);
     operand2 = $bitstoshortreal(B);
     flout_out = (op == 2'b00) ? operand1 + operand2:
              (op == 2'b01) ? operand1 - operand2:
              (op == 2'b10) ? operand1 * operand2:
              operand2;
    gt = (op == 2'b11) ? operand1 > operand2 : 0;
end

endmodule