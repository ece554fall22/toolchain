module hazard_detection_unit
#(
  parameter int DEPTH = 9
)(
  // inputs from current operation in decode
  input vector_wr_en, register_wr_en, mem_stall_in, clk, rst,
  input [5:0] vector_read_register_one, vector_read_register_two,	// for all these bit 5 is 1 if that register is in use else 0
  input [5:0] scalar_read_register_one, scalar_read_register_two,	
  input [4:0] write_register,
  input [1:0] op_type,							

  // outputs go to varous parts of core, stall signals go directly to pipeline registers, fwd signals are pipelined
  output stall_fetch, stall_decode, stall_execute, stall_mem, vector_wb_sel, register_wb_sel,
  output [1:0] ex_to_ex, mem_to_mem, mem_to_ex,
 
  // buffer outputs go directly to writeback controller and tell it to buffer it's vector pipeline results
  output buffer_register_sel, buffer_vector_sel, buffer_register, buffer_vector
);
  // outputs of the vector pipeline fifo containing data on it's operations
  logic [1:0] op_types_v [DEPTH-1:0];
  logic vector_wr_ens_v [DEPTH-1:0];
  logic register_wr_ens_v [DEPTH-1:0];
  logic [4:0] write_registers_v [DEPTH-1:0];

  // data on the scalar pipeline operations
  logic [1:0] op_types_s [1:0];
  logic vector_wr_ens_s [1:0];
  logic register_wr_ens_s [1:0];
  logic [4:0] write_registers_s [1:0];

  // cmp signals are 1 if there is a valid instruction writing to that valid input of the current decode instr
  logic [DEPTH+1:0] v1cmp, v2cmp, sc1cmp, sc2cmp;

  // vector_decode_stall means current instruction goes to the vector pipeline and there exists some instruction in any pipeline writing to one of it's inputs   
  // partial_stall means that only decode and fetch need to stall
  // full_stall means everything but wb needs to be stalled
  logic vector_decode_stall, partial_stall, full_stall;

  // mem_to_ex_hazard means that one instruction is a mem read and the very next instruction uses that value, a stall is inserted in this case
  // stall_on_vector_pipeline means the current instruction of any kind in decode cannot proceed since it has a dependency in the vector pipeline (which cannot forward)
  logic mem_to_ex_hazard, stall_on_vector_pipeline, waw_stall;

  // newer means that the vector operation about to be written back is newer than the scalar result and should wait for the scalar result
  // next buffer sel for vector and scalar wbs are used since this value is pipelined
  logic newer, next_buffer_register_sel, next_buffer_vector_sel, buffer_register_sel_reg, buffer_vector_sel_reg;

  // one bit gets inserted into this
  // 9 deep fifo when the scalar pipeline stalls
  // used to determine whether vector pipeline result
  // is more recent than scalar pipeline result
  logic [DEPTH-1:0] vector_stall_counter; 		

  // is 1 at any position if that spot in the vector pipeline contains something writing to the same place as the scalar pipeline, and it is older (scalar pipeline should stall)
  logic [DEPTH-1:2] waw_hazards, vector_older, vector_same_age, w_cmps;

  // 1 if a register/vector is currently buffered, 
  logic buffered_register, buffered_vector;
  
  // this fifo holds the metadata about all instructions in the vector pipeline
  hazard_fifo #(.DEPTH(DEPTH))
            fifo_vector(.clk(clk), .rst(rst), .op_type(op_type), .write_register(write_register),
                 .vector_wr_en(vector_wr_en & op_type[1] & ~stall_decode), 
                 .register_wr_en(register_wr_en & op_type[1] & ~stall_decode), 
                 .en(1'b1), 
                 .op_types(op_types_v), 
                 .vector_wr_ens(vector_wr_ens_v), 
                 .register_wr_ens(register_wr_ens_v), 
                 .write_registers(write_registers_v));
  
  // this fifo holds metadata bout the execute and mem stages of the scalar pipeline
  hazard_fifo #(.DEPTH(2))
            fifo_scalar(.clk(clk), .rst(rst), .op_type(op_type), .write_register(write_register),
                 .vector_wr_en(vector_wr_en & ~op_type[1] & ~stall_decode), 
                 .register_wr_en(register_wr_en & ~op_type[1] & ~stall_decode), 
                 .en(~full_stall), 
                 .op_types(op_types_s), 
                 .vector_wr_ens(vector_wr_ens_s), 
                 .register_wr_ens(register_wr_ens_s), 
                 .write_registers(write_registers_s));

  // scalar forward paths
  assign ex_to_ex[1] = sc2cmp[0] & ~op_type[1] & ~op_types_s[0][0];
  assign ex_to_ex[0] = sc1cmp[0] & ~op_type[1] & ~op_types_s[0][0];
  assign mem_to_mem[1] = sc2cmp[0] & ~op_type[1] & op_types_s[0][0];
  assign mem_to_mem[0] = sc1cmp[0] & ~op_type[1] & op_types_s[0][0];
  assign mem_to_ex[1] = sc2cmp[1] & ~op_type[1] & ~ex_to_ex[1];
  assign mem_to_ex[0] = sc1cmp[1] & ~op_type[1] & ~ex_to_ex[0];

  // if one op is a read and the next needs that result in execute stage this stall occurs
  assign mem_to_ex_hazard = (sc1cmp[0] | sc2cmp[0]) & ~op_type[0] & ~op_type[1] & op_types_s[0][0];


  // the following compares the current read registers in use to all of the registers being written in both pipelines
  // if a valid comparison is found that bit is set to 1
  genvar i;
  generate
  for (i = 0; i < DEPTH + 2; i++) begin
    if(i < 2) begin
        assign v1cmp[i] = (write_registers_s[i]==vector_read_register_one[4:0]) & vector_read_register_one[5] & vector_wr_ens_s[i];
        assign v2cmp[i] = (write_registers_s[i]==vector_read_register_two[4:0]) & vector_read_register_two[5] & vector_wr_ens_s[i];
        assign sc1cmp[i] = (write_registers_s[i]==scalar_read_register_one[4:0]) & scalar_read_register_one[5] & register_wr_ens_s[i];
        assign sc2cmp[i] = (write_registers_s[i]==scalar_read_register_two[4:0]) & scalar_read_register_two[5] & register_wr_ens_s[i];
    end else begin
        assign v1cmp[i] = (write_registers_v[i-2]==vector_read_register_one[4:0]) & vector_read_register_one[5] & vector_wr_ens_v[i-2];
        assign v2cmp[i] = (write_registers_v[i-2]==vector_read_register_two[4:0]) & vector_read_register_two[5] & vector_wr_ens_v[i-2];
        assign sc1cmp[i] = (write_registers_v[i-2]==scalar_read_register_one[4:0]) & scalar_read_register_one[5] & register_wr_ens_v[i-2];
        assign sc2cmp[i] = (write_registers_v[i-2]==scalar_read_register_two[4:0]) & scalar_read_register_two[5] & register_wr_ens_v[i-2];
    end
  end
endgenerate
  
  // DEPTH long array of single flops that align with the vector pipeline and are 1 if during that cycle of the vector pipeline's execution the scalar pipeline was stalled
  // this is needed to determine which instruction in wb is the newest and should be written back first.
  always_ff @(posedge clk, posedge rst) begin
    for (integer i = 0; i < DEPTH; i++) begin
      if(rst) begin
        vector_stall_counter[i] <= 0;
      end
      else begin
        vector_stall_counter[i] <= (i==0) ? full_stall : vector_stall_counter[i-1];
      end
    end
  end
 
  // this only works if dsp tile delay is 3 maybe? look it over again and retest with different depth if depth changes
  // newer means that the mem stage instr is newer than the instr about to be written back in the vector pipeline
  assign newer = &vector_stall_counter[DEPTH-2:0];

  // this block compares the scalar pipelines next wb (mem) to all wb's in the vector pipeline (that can be older) and basically says if there is an
  // older instruction that has the same destination then we have a waw hazard at that location
  generate
  for (i = 2; i < DEPTH; i++) begin
    assign w_cmps[i] = (write_registers_v[i]==write_registers_s[1]) & ((register_wr_ens_v[i] & register_wr_ens_s[1]) | (vector_wr_ens_v[i] & vector_wr_ens_s[1]));
    assign waw_hazards[i] = w_cmps[i] & vector_older[i];
    if (i>2) begin
      assign vector_older[i] = vector_older[i-1] | (vector_same_age[i-1] & ~vector_stall_counter[i-1]);
      assign vector_same_age[i] = vector_same_age[i-1] & vector_stall_counter[i-1] | (~vector_older[i-1] & ~vector_same_age[i-1] & ~vector_stall_counter[i-1]);
    end 
  end
endgenerate
  
  // vector_same_age and vector_older both have their least significant bit at 2 so this is initial values, and if any bit of waw_hazards is 1 then waw_hazard should go high
  // and the scalar pipeline should be stalled
  assign vector_same_age[2] = ^vector_stall_counter[1:0];
  assign vector_older[2] = ~|vector_stall_counter[1:0];
  assign waw_stall = |waw_hazards;  

  // buffers the register and vector selects that go into the wb controller
  always_ff @(posedge clk, posedge rst) begin
    if(rst) begin 
      buffered_register <= 0;
      buffered_vector <= 0;
      buffer_register_sel_reg <= 0;
      buffer_vector_sel_reg <= 0;
    end else begin
      if(full_stall) begin
        buffered_register <= buffer_register | buffered_register;
        buffered_vector <= buffer_vector | buffered_vector;
        buffer_register_sel_reg <= 0;
        buffer_vector_sel_reg <= 0;
      end else begin
        buffered_register <= 0;
        buffered_vector <= 0;
        buffer_register_sel_reg <= next_buffer_register_sel;
        buffer_vector_sel_reg <= next_buffer_vector_sel;
      end
    end
  end
  
  assign buffer_register_sel = buffer_register_sel_reg;
  assign buffer_vector_sel = buffer_vector_sel_reg;

  // buffers the vector pipelines results if the vector pipeline needs to be written back after the scalar pipeline result
  assign buffer_register = newer & register_wr_ens_v[DEPTH-1] & register_wr_ens_s[1];
  assign next_buffer_register_sel = buffer_register | buffered_register;
  assign buffer_vector =  newer & vector_wr_ens_v[DEPTH-1] & vector_wr_ens_s[1];
  assign next_buffer_vector_sel = buffer_vector | buffered_vector;

  // chooses whether the scalar pipeline or vector pipeline (including bufferred and non-bufferred) get to use the wb stage this cycle
  assign vector_wb_sel = (vector_wr_ens_v[DEPTH-1] & ~buffer_vector) | buffer_vector_sel;
  assign register_wb_sel = (register_wr_ens_v[DEPTH-1] & ~buffer_register) | buffer_register_sel;
  
  // vector_using_wb_stall is 1 if the scalar pipeline should stall because the vector pipeline is using the wb stage that the scalar pipeline wants to use
  assign vector_using_wb_stall = ((vector_wb_sel | buffer_vector_sel_reg) & vector_wr_ens_s[1]) | ((register_wb_sel | buffer_register_sel_reg) & register_wr_ens_s[1]);

  // full_stall occurs on memory stalls, waw stalls, and stalls relating to the vector pipeline needing the wb stage.
  // partial_stalls occur when the current instruction cannot be executed because older instructions need to finish first and there is no valid
  // forward path to allow the instruction to continue this cycle.
  assign stall_fetch = partial_stall | full_stall;
  assign stall_decode = partial_stall | full_stall;
  assign stall_execute = full_stall;
  assign stall_mem = full_stall;

  // assigns full_stall and partial_stall described above
  assign full_stall = vector_using_wb_stall | mem_stall_in | waw_stall;
  assign partial_stall = vector_decode_stall | stall_on_vector_pipeline | mem_to_ex_hazard;

  // stall_on_vector_pipeline means that the vector pipeline contains a result that the current instruction needs
  // vector_decode_stall means that the current operation is a vector operation therefore any dependency must be stalled on, including a scalar pipeline dependency
  assign stall_on_vector_pipeline = (|v1cmp[DEPTH+1:2]) | (|v2cmp[DEPTH+1:2]) | (|sc1cmp[DEPTH+1:2]) | (|sc2cmp[DEPTH+1:2]) | buffer_vector_sel | buffer_register_sel | buffered_vector | buffered_register;
  assign vector_decode_stall = ((|v1cmp) | (|v2cmp)) | (((|sc1cmp) | (|sc2cmp)) & op_type[1]);
  

endmodule
