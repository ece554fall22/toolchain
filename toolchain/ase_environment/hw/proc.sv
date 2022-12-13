module proc(
    input clk, rst,
    output err
);

control_bus fetch_control(); 
control_bus decode_control();
control_bus s_execute_control();
control_bus s_memory_control();
control_bus s_writeback_control();
control_bus vector_execute_control_0();
control_bus vector_execute_control_1();
control_bus vector_execute_control_2();
control_bus vector_execute_control_3();
control_bus vector_execute_control_4();
control_bus vector_execute_control_5();
control_bus vector_execute_control_6();
control_bus vector_execute_control_7();
control_bus vector_execute_control_8();
control_bus v_writeback_control();

logic rst_n;

assign rst_n = !rst;

logic [35:0] branch_pc;

logic [31:0] inst_f, inst_d;

logic [35:0] pc_f, pc_d, pc_e, pc_m, pc_w;

logic [35:0] register_write_data;

logic [31:0] vector_write_data [3:0];

logic [4:0] vread1, vread2;

logic zero, sign, overflow;

logic [31:0] vdata1_d [3:0], vdata2_d [3:0], vdata1_e [3:0], vdata2_e [3:0], vdata_out_e [3:0], matmul_data_out_e [3:0], matmul_data_out_w [3:0], vdata_out_w [3:0];

logic [35:0] sdata1_d, sdata2_d, sdata1_e, sdata2_e, sdata_out_e, sdata_out_m, sdata_out_w, fdata_out_e, fdata_out_w;

logic [7:0] vector_imm_d, vector_imm_e;

logic [24:0] immediate_d, immediate_e, immediate_m, immediate_w;

fetch fetch (.clk(clk), .rst(rst), .pc_control(fetch_control.pc_select), .stall(1'b0), .halt(fetch_control.halt), 
.unhalt(), .cache_stall(), .pc_branch(branch_pc), .instr(inst_f), .pc_next(pc_f), .is_running(), .vread1(vread1), .vread2(vread2));



decode decode(.clk(clk), .rst_n(!rst), .s_wr_en(s_writeback_control.register_wr_en), .inst(inst_d), .pc_plus_4(register_write_data), .s_write_data(register_write_data), 
.v_read1(vread1), .v_read2(vread2), .v_write_addr(v_writeback_control.vector_write_register), 
.r_write_addr(s_writeback_control.scalar_write_register), .r_read1(inst_f[19:15]), .r_read2(inst_f[14:10]), .write_vector(vector_write_data), 
.mask(v_writeback_control.mask), .zero(zero), .sign(sign), .overflow(overflow), .control(decode_control), .vdata1(vdata1_d), .vdata2(vdata2_d), .sdata1(sdata1_d), .sdata2(sdata2_d), .immediate(immediate_d), .pc_next(branch_pc));

assign vector_imm_d = inst_d[14:7];

scalar_execute sexecut (.clk(clk), .rst_n(!rst), .data1(sdata1_e), .data2(sdata2_e), .control(s_execute_control), 
.immediate(immediate_e), .zero(zero), .sign(sign), .overflow(overflow), .data_out(sdata_out_e));

vector_execute vexecute(.clk(clk), .rst_n(!rst), .vdata1(vdata1_e), .vdata2(vdata2_e), .data1(sdata1_e[31:0]), .data2(sdata2_e[31:0]), .immediate(vector_imm_e), .control(vector_execute_control_0), .vdata_out(vdata_out_e), .rdata_out(fdata_out_e));

//tpuv1 (.clk(clk), .rst_n(!rst), .hl(execute_control.matmul_high_low), .v_high(vdata1_e), .v_low(vdata2_e), .idx(execute_control.matmul_idx), .opcode(execute_control.matmul_opcode), .data_out(matmul_data_out_e));


logic [35:0] smem_data_m, smem_data_w;

logic [31:0] vmem_data_m [3:0], vmem_data_w [3:0];

//mem memory (.clk(clk), .rst(rst), .line(), .w_type(s_memory_control.w_type), .cache_data(), .alu_data(sdata_out_m), .mem_operation(s_memory_control.r_type), .register_wb(smem_data_m), .vector_wb({vmem_data_m[3], vmem_data_m[2], vmem_data_m[1], vmem_data_m[0]}));

logic [35:0] swb_data;

assign swb_data = (s_writeback_control.mem_read) ? smem_data_w : 
                  (s_writeback_control.store_immediate) ? immediate_w :
                   sdata_out_w;

wb writeback(.scalar_pipeline_wb(swb_data), .vector_pipeline_wb(fdata_out_w), .pc(pc_w), .scalar_pipeline_vwb({vmem_data_w[3], vmem_data_w[2], vmem_data_w[1], vmem_data_w[0]}), .vector_pipeline_vwb({vdata_out_w[3], vdata_out_w[2], vdata_out_w[1], vdata_out_w[0]}), 
.scalar_pipeline_we(s_writeback_control.register_wr_en), .vector_pipeline_we(v_writeback_control.vector_wr_en), .pc_sel(s_writeback_control.store_pc), .scalar_pipeline_mask(s_writeback_control.mask), .vector_pipeline_mask(v_writeback_control.mask),
.register_wb_sel(1'b0), .vector_wb_sel(1'b0), .buffer_register_sel(1'b0), .buffer_vector_sel(1'b0), .buffer_register(1'b0), .buffer_vector(1'b0),
.vector_we(), .register_we(), .vector_data({vector_write_data[3], vector_write_data[2], vector_write_data[1], vector_write_data[0]}), .register_data(register_write_data), .clk(clk), .rst(rst), .scalar_pipeline_wbr(s_writeback_control.scalar_write_register),
.vector_pipeline_wbr(v_writeback_control.vector_write_register), .vector_wbr(), .register_wbr());



control_pipeline df(.clk(clk), .rst_n(!rst), .control_q(fetch_control), .control_d(decode_control));
control_pipeline de(.clk(clk), .rst_n(!rst), .control_q(s_execute_control), .control_d(decode_control));
control_pipeline em(.clk(clk), .rst_n(!rst), .control_q(s_memory_control), .control_d(s_execute_control));
control_pipeline mw(.clk(clk), .rst_n(!rst), .control_q(s_writeback_control), .control_d(s_memory_control));



control_pipeline ve_1(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_1), .control_d(vector_execute_control_0));
control_pipeline ve_2(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_2), .control_d(vector_execute_control_1));
control_pipeline ve_3(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_3), .control_d(vector_execute_control_2));
control_pipeline ve_4(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_4), .control_d(vector_execute_control_3));
control_pipeline ve_5(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_5), .control_d(vector_execute_control_4));
control_pipeline ve_6(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_6), .control_d(vector_execute_control_5));
control_pipeline ve_7(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_7), .control_d(vector_execute_control_6));
control_pipeline ve_8(.clk(clk), .rst_n(!rst), .control_q(vector_execute_control_8), .control_d(vector_execute_control_7));
control_pipeline ve_9(.clk(clk), .rst_n(!rst), .control_q(v_writeback_control), .control_d(vector_execute_control_8));

always_ff @(posedge clk, negedge rst_n) begin
    if(!rst_n) begin
        inst_d <= '0;
        pc_d <= '0;
        pc_e <= '0;
        pc_m <= '0;
        pc_w <= '0;
        sdata1_e <= '0;
        sdata2_e <= '0;
        sdata_out_m <= '0;
        sdata_out_w <= '0;
        fdata_out_w <= '0;
        vector_imm_e <= '0;
        immediate_e <= '0;
        immediate_m <= '0;
        immediate_w <= '0;
    end
    else begin
        inst_d <= inst_f;
        pc_d <= pc_f;
        pc_e <= pc_d;
        pc_m <= pc_m;
        pc_w <= pc_w;
        sdata1_e <= sdata1_d;
        sdata2_e <= sdata2_d;
        sdata_out_m <= sdata_out_e;
        sdata_out_w <= sdata_out_m;
        fdata_out_w <= fdata_out_e;
        vector_imm_e <= vector_imm_d;
        immediate_e <= immediate_d;
        immediate_m <= immediate_e;
        immediate_w <= immediate_m;
    end
end

genvar i;
generate;
    for(i = 0; i < 4; i ++) begin
        always_ff @(posedge clk, negedge rst_n) begin
            if(!rst_n) begin
                vdata1_e[i] <= '0;
                vdata2_e[i] <= '0;
                vdata_out_w[i] <= '0;
                matmul_data_out_w[i] <= '0;
            end
            else begin
                vdata1_e[i] <= vdata1_d[i];
                vdata2_e[i] <= vdata1_d[i];
                vdata_out_w[i] <= vdata_out_e[i];
                matmul_data_out_w[i] <= matmul_data_out_e[i];
            end
        end
    end
endgenerate
endmodule