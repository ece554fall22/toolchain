module ram_single(q, a, d, we, clk);
  parameter REG_SIZE = 36;
  parameter FILE_SIZE = 32;
  parameter ADDR_SIZE = 5;
  output logic [REG_SIZE -1:0] q;
  input [REG_SIZE -1:0] d;
  input [ADDR_SIZE -1:0] a;
  input we, clk;
  reg [REG_SIZE -1:0] mem [FILE_SIZE-1:0];
  always_ff @(posedge clk) begin 
    q <= mem[a];
  end 
  always_ff @(negedge clk) begin
    if (we)
      mem[a] <= d;
  end
endmodule
