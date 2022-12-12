module systolic_array #(
  parameter BITS_AB=32,
  parameter BITS_C=32,
  parameter DIM=8
) (
  input clk, rst_n,
  input en, WrEn, hl,
  input [BITS_AB-1:0] A [DIM-1:0],
  input [BITS_AB-1:0] B [DIM-1:0],
  input [BITS_C-1:0]  Cin [DIM-1:0],
  input [$clog2(DIM)-1:0]    Crow,
  output logic [BITS_C-1:0] Cout [DIM/2-1:0]
);
  genvar row, col;

  // Ains, Bins oversized by 1 in dimension of propagation to avoid special-casing in generate;
  // those wires are just stubs that will optimize out
  logic [BITS_AB-1:0] Ains  [DIM-1:0] [DIM:0];
  logic  [BITS_AB-1:0] Bins  [DIM:0] [DIM-1:0];
  logic[BITS_C-1:0]  Couts [DIM-1:0] [DIM-1:0];

  generate
    // first row/col A/B inputs: feed from A, B
    for (row = 0; row < DIM; row++) begin : Ain_feed_wires
      assign Ains[row][0] = A[row];
    end
    for (col = 0; col < DIM; col++) begin : Bin_feed_wires
      assign Bins[0][col] = B[col];
    end

    // instantiate grid of MAC units
    for (row=0; row<DIM; row++) begin : systolic_row
      for (col=0; col<DIM; col++) begin : systolic_col
        tpumac #(
          .BITS_AB(BITS_AB),
          .BITS_C(BITS_C)
        ) mac (
          .clk(clk),
          .rst_n(rst_n),

          .en   (en),
          .WrEn (WrEn && (Crow === row)),

          .Ain  (Ains[row][col]),
          .Bin  (Bins[row][col]),
          .Cin  (Cin[col]),

          .Aout (Ains[row][col+1]),
          .Bout (Bins[row+1][col]),
          .Cout (Couts[row][col])
        );
      end
    end
  endgenerate

  // demuxing for C outputs
  assign Cout = (hl) ? Couts[Crow][7:4] : Couts[Crow][3:0];
endmodule // systolic_array
