module fetch
(
  input logic clk, rst, pc_control, stall, halt, unhalt, cache_stall,
  input logic [35:0] pc_branch,
  input logic [31:0] instr,
  output [35:0] pc_next,
  output is_running,
  output[4:0] vread1, vread2
);

// intermediate value to calculate vread1
logic vread1_inter;

// pc_next_mux is the output of the mux determining pc_next
logic [35:0] pc_next_mux, pc_plus_4, pc;

// halt_reg is output of flop that stores whether core is halted or not
logic halt_reg, next_halt_reg;

// decides if pc should branch or not
assign pc_next_mux = (pc_control) ? pc_branch : pc_plus_4;

// causes pc to retain it's value if stall is asserted
assign pc_next = (stall | cache_stall) ? pc : pc_next_mux;

// is_running is high when the core is not halted
assign is_running = ~halt_reg;

// this is adding 4 to the pc
assign pc_plus_4 = pc + 4;

// decides if halt is high next cycle
assign next_halt_reg = (halt_reg & ~unhalt) | (~halt_reg & halt);

// pc register
always_ff @(posedge clk, posedge rst) begin
  if(rst)
    pc <= 0;				
  else
    pc <= pc_next;
end

// stores if core is halted or not
always_ff @(posedge clk, posedge rst) begin
  if(rst)
    halt_reg <= 1'b0;			
  else
    halt_reg <= next_halt_reg;
end

// intermediate signal to decode vread1
assign vread1_inter = (instr[31:25]==7'b0100100) |  // vdota
		     (instr[31:25]==7'b0101101) |  // vsma
		     (instr[31:25]==7'b0110110) |  // vcomp
		     (instr[31:25]==7'b0010010);   // vstr

// decodes the first read vector
assign vread1 = (vread1_inter) ? instr[9:5] : instr[19:15];

// "decodes" the second read register
assign vread2 = instr[14:10];

endmodule
