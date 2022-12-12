module decode(
    input clk, rst_n, s_wr_en,
    input [31:0] inst,
    input [35:0] pc_plus_4, write_data,
    input [4:0] v_read1, v_read, v_write_addr, r_write_addr, r_read1, r_read2,
    input [31:0] write_vector [3:0],
    input [3:0] mask,
    input zero, sign, overflow,
    control_bus control,
    output logic [31:0] vdata1 [3:0],
    output logic [31:0] vdata2 [3:0],
    output logic [35:0] sdata1, sdata2,
    output logic [24:0] immediate,
    output logic [35:0] pc_next
);

always_comb begin
    case(control.imm_type)
        4'b0001: immediate = inst[24:0];
        4'b0001: immediate = {{4{inst[24]}},inst[24:20], inst[15:0]};
        4'b0010: immediate = {{3{inst[21]}}, inst[21:0]};
        4'b0011: immediate = {{10{inst[14]}}, inst[14:0]};
        4'b0100: immediate = {{6{inst[18]}}, inst[18:0]};
        4'b0101: immediate = {{10{inst[24]}}, inst[24:20], inst[9:0]};
        4'b0110: immediate = {{14{inst[14]}}, inst[14:4]};
        4'b0111: immediate = {{10{inst[24]}}, inst[24:20], inst[14:4]};
        4'b1000: immediate = {{14{inst[24]}}, inst[24:20], inst[9:4]};
    endcase
end




vector_register_file vregs(.read1(v_read1), .read2(v_read2), .write_addr(write_addr), .write_vector(write_vector), .data1(vdata1), .data2(vdata2), .we(mask), .clk(clk), .rst_n(rst_n));

register_file sregs (.read1(r_read1), .read2(r_read2), .write_addr(write_addr), write_data(write_data), .we(s_wr_en), .data1(sdata1), .data2(sdata2), .clk(clk), .rst_n(rst_n));

control_unit control_decode (.inst(inst), .control(control));

branch_jump_decoder bjdec (.branch_jump(control.branch_jump), .branch_type(inst[24:22]), .branch_register(control.branch_register), .immediate(immediate), .register(sdata1), .pc(pc_plus_4), .zero(zero), .sign(sign), .overflow(overflow), .pc_next(pc_next));

endmodule


