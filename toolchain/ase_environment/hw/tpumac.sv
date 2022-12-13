module tpumac #(
  parameter int BITS_AB = 32,
  parameter int BITS_C = 32 // n.b.: BITS_C should be BITS_AB*2. could do an `initial assert`; shame SV lacks a static_assert.
) (
  input logic clk, rst_n,
  input logic en,
  input logic WrEn,

  input logic [BITS_AB-1:0] Ain,
  input logic [BITS_AB-1:0] Bin,
  input logic [BITS_C-1:0]  Cin,

  output logic [BITS_AB-1:0] Aout,
  output logic [BITS_AB-1:0] Bout,
  output logic [BITS_C-1:0]  Cout
);

  // Aout, Bout registers
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      Aout <= '0;
      Bout <= '0;
    end else if (en) begin
      Aout <= Ain;
      Bout <= Bin;
    end
  end

  shortreal operand1, operand2, A_mul_B, A_mul_B_plus_Cout, accum, cinput;

  always_comb begin
     cinput = $bitstoshortreal(Cin);
     operand1 = $bitstoshortreal(Ain);
     operand2 = $bitstoshortreal(Bin);
     A_mul_B = operand1 * operand2;
     A_mul_B_plus_Cout = A_mul_B + accum;
end

  // Cout register
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      accum <= '0;
    end else if (en || WrEn) begin
      // when WrEn, we're loading a new value of C
      accum <= cinput ? Cin : A_mul_B_plus_Cout;
    end
  end
  assign Cout = accum;
endmodule // tpumac
