interface control_bus;
    logic vector_wr_en, register_wr_en, matmul_high_low, synch_req, branch_register, mem_read, mem_write, halt, matrix_mutplier_en, flushicache, store_pc, alu_operands, r_read1, r_read2, v_read1, v_read2, dirty, clean, pc_select, store_immediate;
    logic [4:0] vector_read_register1, vector_read_register2, scalar_read_register1, scalar_read_register2, scalar_write_register, vector_write_register,vector_alu_op;
    logic [1:0] op_type, w_type, r_type, scalar_op_sel, synch_op, data_cache_flush, branch_jump;
    logic [3:0] scalar_alu_op, imm_type, mask;
    logic [2:0] matmul_opcode, matmul_idx;    
endinterface