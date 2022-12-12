module hazard_fifo #(
  parameter int DEPTH = 9
 ) (
  input logic clk, rst, en,
  input logic [1:0] op_type,
  input logic [4:0] write_register,
  input logic vector_wr_en, register_wr_en,

  // for scalar pipeline bit 0 1 = mem instr
  // for vector pipeline bit 0 1 = 3fp alu 0 = 1
  // bit 1 is 1 for vector pipeline 0 for scalar pipeline
  output logic [1:0] op_types [DEPTH-1:0],

  // wr_en signals to vector register file
  output logic vector_wr_ens [DEPTH-1:0],

  // wr_en signals to scalar register file
  output logic register_wr_ens [DEPTH-1:0],

  // write registers
  output logic [4:0] write_registers [DEPTH-1:0]
 );

  // essentially a fifo of meta data about instructions
  // all segments of the fifo are needed as outputs since the hazard detection unit
  // needs to look for hazards in all instructions
  always_ff @(posedge clk, posedge rst) begin
    for (integer i = 0; i < DEPTH; i++) begin
      if(rst) begin
        op_types[i] <= 0;
	vector_wr_ens[i] <= 0;
	register_wr_ens[i] <= 0;
	write_registers[i] <= 0;
      end
      else if (en) begin
        op_types[i] <= (i==0) ? op_type : op_types[i-1];
	vector_wr_ens[i] <= (i==0) ? vector_wr_en : vector_wr_ens[i-1];
	register_wr_ens[i] <= (i==0) ? register_wr_en : register_wr_ens[i-1];
	write_registers[i] <= (i==0) ? write_register : write_registers[i-1];
      end
    end
  end

endmodule
