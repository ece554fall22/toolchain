module branch_jump_decoder(branch_jump, branch_type, branch_register, immediate, register, pc, nz, ez, lz, gz, le, ge, pc_next);
input [1:0] branch_jump;
input [2:0] branch_type;
input branch_register, zero, sign, overflow;
input [24:0] immediate;
input [35:0] register;
input [35:0] pc;
output logic [35:0] pc_next;

logic [35:0] branch;

logic nz, ez, lz, gz, le, ge;

assign nz = !zero;
assign ez = zero;
assign lz = sign & !overflow;
assign gz = !sign & !overflow;
assign le = lz | ez;
assign gz = gz | ez;

always_comb begin
    branch = (branch_type == 3'b000) ? ((nz) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             (branch_type == 3'b001) ? ((ez) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             (branch_type == 3'b010) ? ((lz) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             (branch_type == 3'b011) ? ((gz) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             (branch_type == 3'b100) ? ((le) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             (branch_type == 3'b101) ? ((ge) ? ((branch_register) ? pc + register + immediate: pc + immediate) : pc):
             pc;
    pc_next = (branch_jump == 2'b01) ? ((branch_register) ? pc + register + immediate : pc + immediate) :
              (branch_jump == 2'b10) ? branch: pc;
end



endmodule
