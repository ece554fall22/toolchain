module control_unit(input [31:0] inst, control_bus control);

enum {halt, nop, jmp, jal, jmpr, jalr, bi, br, lih, lil, ld32, ld36, st32, st36, vldi, vsti, vldr, vstr, Addi, Subi, Andi, Ori, xori, shli, shri, cmpi, alu, Not, Falu, cmp, Vadd, Vsub,
Vmult, Vdiv, Vdot, Vdota, Vindx, Vreduce, Vsplat, Vswizzle, Vsadd, Vsmult, Vssub, Vsdiv, vsma, writeA, writeB, writeC, matmul, readC,
systolicstep, Vmax, Vmin, Vcomp, ftoi, itof, wcsr, rcsr, fa, cmpx, flushdirty, flushclean, flushicache, flushline, cmpdec, cmpinc} oppcode;

enum {FFadd, FFsub, FFmult, VVadd, VVsub, VVmult, VVdot, VVdota, VVindx, VVreduce, VVsplat, VVswizzle, VVsadd,VVssub, VVsmult, VVsma, VVcompsel, VVmax, VVmin} operations;
logic [6:0] op_code;

assign op_code = inst[31:25];

always_comb begin
    control.halt = 0;
    control.flushicache = 0;
    control.data_cache_flush = '0;
    control.dirty = 0;
    control.clean = 0;
    control.matrix_mutplier_en = 0;
    control.vector_wr_en = 0;
    control.register_wr_en = 0;
    control.mem_read = 0;
    control.mem_write = 0;
    control.vector_read_register1 = inst[14:10];
    control.vector_read_register2 = inst[19:15];
    control.scalar_read_register1 = inst[19:15];
    control.scalar_read_register2 = inst[14:10];
    control.vector_write_register = inst[24:20];
    control.scalar_write_register = inst[24:20];
    control.vector_alu_op = '0;
    control.op_type = '0;
    control.w_type = '0;
    control.r_type = '0;
    control.scalar_op_sel = '0;
    control.synch_op = '0;
    control.branch_control = '0;
    control.matmul_idx = '0;
    control.matmul_opcode = '0;
    control.matmul_high_low = 0;
    control.synch_req = 0;
    control.pc_select = 0;
    control.branch_jump = '0;
    control.branch_register = 0;
    control.store_pc = 0;
    control.alu_operands = 0;
    control.imm_type = '0;
    control.r_read1 = 0;
    control.r_read2 = 0;
    control.v_read1 = 0;
    control.v_read2 = 0;
    control.store_immediate = 0;
    control.mask = inst[3:0];
    case (op_code)
        halt: begin
            control.halt = 1;
        end
        nop: begin
        end
        jmp: begin
            control.branch_jump = 2'b01;
            control.pc_select = 1;
        end
        jal: begin
            control.branch_jump = 2'b01;
            control.register_wr_en = 1;
            control.scalar_write_register = 32'hFFFF;
            control.store_pc = 1;
            control.pc_select = 1;
        end
        jmpr: begin
            control.branch_jump = 2'b01;
            control.branch_register = 1;
            control.imm_type = 4'b0001;
            control.r_read1 = 1;
            control.pc_select = 1;
        end
        jalr: begin
            control.branch_jump = 2'b01;
            control.branch_register = 1;
            control.register_wr_en = 1;
            control.scalar_write_register = 32'hFFFF;
            control.store_pc = 1;
            control.imm_type = 4'b0001;
            control.r_read1 = 1;
            control.pc_select = 1;
        end
        bi: begin
            control.branch_jump = 2'b10;
            control.imm_type = 4'b0010;
            control.pc_select = 1;
        end
        br: begin
            control.branch_jump = 2'b10;
            control.branch_register = 1;
            control.imm_type = 4'b0011;
            control.r_read1 = 1;
            control.pc_select = 1;
        end
        lih: begin
            control.register_wr_en = 1;
            control.imm_type = 4'b0100;
            control.store_immediate = 1;
        end
        lil: begin
            control.register_wr_en = 1;
            control.imm_type = 4'b0100;
            control.store_immediate = 1;
        end
        ld32: begin
            control.register_wr_en = 1;
            control.mem_read = 1;
            control.r_type = 2'b01;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0011;
            control.r_read1 = 1;
        end
        ld36: begin
            control.register_wr_en = 1;
            control.mem_read = 1;
            control.r_type = 2'b10;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0011;
            control.r_read1 = 1;
        end
        st32: begin
            control.mem_write = 1;
            control.w_type = 2'b01;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0101;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        st36: begin
            control.mem_write = 1;
            control.w_type = 2'b10;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0101;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        vldi: begin
            control.mem_read = 1;
            control.vector_wr_en = 1;
            control.register_wr_en = 1;
            control.r_type = 2'b11;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0110;
            control.scalar_write_register = inst[19:15];
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        vsti: begin
            control.mem_write = 1;
            control.vector_wr_en = 1;
            control.register_wr_en = 1;
            control.w_type = 2'b11;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 1;
            control.imm_type = 4'b0110;
            control.scalar_write_register = inst[19:15];
            control.r_read1 = 1;
            control.v_read1 = 1;
        end
        vldr: begin
            control.mem_read = 1;
            control.vector_wr_en = 1;
            control.register_wr_en = 1;
            control.r_type = 2'b11;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 0;
            control.scalar_write_register = inst[19:15];
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        vstr: begin
            control.mem_write = 1;
            control.vector_wr_en = 1;
            control.register_wr_en = 1;
            control.w_type = 2'b11;
            control.scalar_alu_op = 4'b0000;
            control.alu_operands = 0;
            control.scalar_write_register = inst[19:15];
            control.r_read1 = 1;
            control.r_read2 = 1;
            control.v_read1 = 1;
        end
        Addi: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0000;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        Subi: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0001;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        Andi: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0010;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        Ori: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0011;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        xori: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0100;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        shli: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0101;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        shri: begin
            control.register_wr_en = 1;
            control.alu_operands = 1;
            control.scalar_alu_op = 4'b0111;
            control.imm_type = 4'b0110;
            control.r_read1 = 1;
        end
        cmpi: begin
            control.alu_operands = 1;
            contol.scalar_alu_op = 4'b1000;
            control.imm_type = 4'b0111;
            control.r_read1 = 1;
        end
        alu: begin
            control.register_wr_en = 1;
            control.scalar_alu_op = inst[3:0];
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        Not: begin
            control.register_wr_en = 1;
            control.r_read1 = 1;
        end
        Falu: begin
            control.register_wr_en = 1;
            control.vector_alu_op = {2'b0, inst[2:0]};
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        cmp: begin
            control.scalar_alu_op = 4'b1000;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        Vadd: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVadd;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        Vsub: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVsub;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        Vmult: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVmult;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        Vdiv: begin
        end
        Vdot: begin
            control.register_wr_en = 1;
            control.vector_alu_op = VVdot;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        Vdota: begin
            control.register_wr_en = 1;
            control.vector_alu_op = VVdota;
            control.vector_read_register1 = inst[14:10];
            control.vector_read_register2 = inst[9:5];
            control.v_read1 = 1;
            control.v_read2 = 1;
            control.r_read1 = 1;
        end
        Vindx: begin
            control.register_wr_en = 1;
            control.vector_alu_op = VVindx;
            control.v_read1 = 1;
        end
        Vreduce: begin
            control.register_wr_en = 1;
            control.vector_alu_op = VVreduce;
            control.v_read1 = 1;
        end
        Vsplat: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVsplat;
            control.v_read1 = 1;
        end
        Vswizzle: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVswizzle;
            control.v_read1 = 1;
        end
        Vsadd: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVsadd;
            control.v_read2 = 1;
            control.r_read1 = 1;
        end
        Vsmult: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVsmult;
            control.v_read2 = 1;
            control.r_read1 = 1;
        end
        Vssub: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVssub;
            control.v_read2 = 1;
            control.r_read1 = 1;
        end
        vsma: begin
            control.vector_wr_en = 1;
            contro.vector_alu_op = VVsma;
            control.vector_read_register1 = inst[14:10];
            control.vector_read_register2 = inst[9:5];
            control.v_read1 = 1;
            control.v_read2 = 1;
            control.r_read1 = 1;
        end
        writeA: begin
            control.matmul_opcode = 3'b001;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        writeB: begin
            control.matmul_opcode = 3'b010;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        writeC: begin
            control.matmul_opcode = 3'b011;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        matmul: begin
            control.matmul_opcode = 3'b100;
        end
        readC: begin
            control.vector_wr_en = 1;
            control.matmul_opcode = 3'b101;
            control.matmul_high_low = inst[16];
        end
        systolicstep: begin
            control.matmul_opcode = 3'b110;
        end
        Vmax: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVmax;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        Vmin: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVmin;
            control.v_read1 = 1;
            control.v_read2 = 1;
        end
        VVcompsel: begin
            control.vector_wr_en = 1;
            control.vector_alu_op = VVcompsel;
            control.v_read1 = 1;
            control.v_read2 = 1;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        ftoi: begin
            control.r_read1 = 1;
        end
        itof: begin
            control.r_read1 = 1;
        end
        wcsr: begin
            control.r_read1 = 1;
        end
        rcsr: begin
            control.r_read1 = 1;
        end
        fa: begin
            control.register_wr_en = 1;
            control.mem_read = 1;
            control.r_type = 2'b10;
            control.r_read1 = 1;
        end
        cmpx: begin
            control.register_wr_en = 1;
            control.mem_read = 1;
            control.r_type = 2'b10;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        flushdirty: begin
            control.data_cache_flush = 1;
            control.dirty = 1;
        end
        flushclean: begin
            control.data_cache_flush = 1;
            control.clean = 1;
        end
        flushicache: begin
            control.flushicache = 1;
        end
        flushline: begin
            control.data_cache_flush = 1;
        end
        cmpdec: begin
            control.scalar_alu_op = 4'b1000;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
        cmpinc: begin
            control.scalar_alu_op = 4'b1000;
            control.r_read1 = 1;
            control.r_read2 = 1;
        end
    endcase
end

endmodule