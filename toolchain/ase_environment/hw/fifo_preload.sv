// fifo.sv
// Implements delay buffer (fifo)
// On reset all entries are set to 0
// Shift causes fifo to shift out oldest entry to q, shift in d

module fifo_preload #(
  parameter int DEPTH=8,
  parameter int BITS=32
) (
  input logic clk,rst_n,en, wr,
  input logic signed [BITS-1:0] d [DEPTH-1:0],
  output logic signed  [BITS-1:0] q
);
  
  logic signed [BITS-1:0] registers [DEPTH*2-2:0];

  // infers an array of chained flops. all flops stop registering their `d` when `~en`.
  always_ff @(posedge clk or negedge rst_n) begin
    for (integer i = 0; i < DEPTH*2-1; i++) begin
      if (!rst_n) begin
        registers[i] <= '0;
      end
      else if(wr) begin
        if(i <= 6) begin
          registers[i] <= '0;
        end
        else begin
          registers[i] <= d[i-DEPTH +1];
        end
      end
      else if(en) begin
        registers[i] <= (i == DEPTH*2-2) ? 0 : registers[i+1];
      end
    end
  end

  assign q = registers[0];
endmodule // fifo_preload
