module tpuv1 #(
  parameter BITS_AB=32,
  parameter BITS_C=32,
  parameter DIM=8,
  parameter ADDRW=16,
  parameter DATAW=64
) (
  input clk, rst_n, hl,
  input [31:0] v_high [3:0],
  input [31:0] v_low [3:0],
  input [3:0] idx,
  input [2:0] opcode,
  output logic [31:0] data_out [3:0]
);
  /*------------------------------------------------------------------------------
  --  memories
  ------------------------------------------------------------------------------*/
  // wiring from memories to systolic array
  wire signed [BITS_AB-1:0] Aout [DIM-1:0];
  wire signed [BITS_AB-1:0] Bout [DIM-1:0];

  logic [32:0] v_in [DIM-1:0];

  genvar i;
  generate
    for(i = 0; i < DIM; i++) begin
      if(i < 4)
        assign v_in[i] = v_low[i];
      else
        assign v_in[i] = v_high[i-4];
    end
  endgenerate
  

  logic memA_en, memA_WrEn;
  memA #(.BITS_AB(BITS_AB), .DIM(DIM)) memory_A (
    .clk  (clk),
    .rst_n(rst_n),
    .en   (memA_en),
    .WrEn (memA_WrEn),
    .Ain  (v_in),
    .Arow (idx),
    .Aout (Aout)
  );

  logic memB_en;
  memB #(.BITS_AB(BITS_AB), .DIM(DIM)) memory_B (
    .clk  (clk),
    .rst_n(rst_n),
    .en   (memB_en),
    .Bin  (v_in),
    .Bout (Bout)
  );

  /*------------------------------------------------------------------------------
  --  systolic array
  ------------------------------------------------------------------------------*/
  logic systolic_WrEn, systolic_en;
  logic [BITS_C-1:0] Cout [DIM/2-1:0];
  systolic_array #(
    .BITS_AB(BITS_AB), .BITS_C(BITS_C), .DIM(DIM)
  ) systolic_arr (
    .clk  (clk),
    .rst_n(rst_n),
    .en   (systolic_en),
    .WrEn (systolic_WrEn),
    .hl   (hl),
    .Crow (idx),
    .Cin  (v_in),
    .Cout (Cout),

    .A    (Aout),
    .B    (Bout)
  );

  /*------------------------------------------------------------------------------
  --  address selection logic
  ------------------------------------------------------------------------------*/
  logic read;
  enum {nop, writeA, writeB, writeC, matmul, readC, systolic_step} opcodes;
  always_comb begin
    // defaults
    memA_en = '0;
    memA_WrEn = '0;
    memB_en = '0;
    systolic_WrEn = '0;
    read = '0;
    systolic_en = 0;
    case(opcode)
      writeA: begin
        memA_en = 1;
        memA_WrEn = 1;
      end
      writeB: begin
        memB_en = 1;
      end
      writeC: begin
        systolic_en = 1;
        systolic_WrEn =1;
      end
      matmul: begin
        memA_en = 1;
        memB_en = 1;
        systolic_en = 1;
      end
      systolic_step: begin
        memA_en = 1;
        memB_en = 1;
        systolic_en = 1;
      end
      readC: begin
        read = 1;
      end
    endcase
  end

  genvar y;
  generate 
    for(y = 0; y < 4; y++) begin
      assign data_out[y] = (read) ? Cout[y] : 32'b0;
    end
  endgenerate

endmodule // tpuv1
