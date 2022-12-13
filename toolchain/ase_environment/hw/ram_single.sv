module ram_single(q, a_in, a_out, d, we, clk1, clk2);
  parameter REG_SIZE = 36;
  parameter FILE_SIZE = 32;
  parameter ADDR_SIZE = 5;
  output logic [REG_SIZE -1:0] q;
  input [REG_SIZE -1:0] d;
  input [ADDR_SIZE -1:0] a_in, a_out;
  input we, clk1, clk2;
  reg [REG_SIZE -1:0] mem [FILE_SIZE-1:0];
  reg [ADDR_SIZE-1:0] addr_out_reg;
  always @(posedge clk1) begin
      if (we)
         mem[a_in] <= d;
   end
 
   always @(posedge clk2) begin
      q <= mem[addr_out_reg];
      addr_out_reg <= a_out;
   end
        
endmodule
