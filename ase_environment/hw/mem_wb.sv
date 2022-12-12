module mem_wb
(
input clk, rst,
input logic [35:0] scalar_pipeline_wb, vector_pipeline_wb, pc,
output logic [35:0] scalar_pipeline_wb_reg, vector_pipeline_wb_reg, pc_reg,
input logic [127:0] scalar_pipeline_vwb, vector_pipeline_vwb,
output logic [127:0] scalar_pipeline_vwb_reg, vector_pipeline_vwb_reg,
input logic [3:0] scalar_pipeline_mask, vector_pipeline_mask,
output logic [3:0] scalar_pipeline_mask_reg, vector_pipeline_mask_reg,
input logic register_wb_sel, vector_wb_sel, pc_sel, scalar_pipeline_we, vector_pipeline_we,
output logic register_wb_sel_reg, vector_wb_sel_reg, pc_sel_reg, scalar_pipeline_we_reg, vector_pipeline_we_reg,
input logic buffer_register_sel, buffer_vector_sel, buffer_register, buffer_vector,
output logic buffer_register_sel_reg, buffer_vector_sel_reg, buffer_register_reg, buffer_vector_reg
);

// it's a pipeline register
always_ff @(posedge clk, posedge rst) begin
  if(rst) begin
    scalar_pipeline_wb_reg <= 0;
    vector_pipeline_wb_reg <= 0;
    pc_reg <= 0;
    scalar_pipeline_vwb_reg <= 0;
    vector_pipeline_vwb_reg <= 0;
    scalar_pipeline_mask_reg <= 0;
    vector_pipeline_mask_reg <= 0;
    register_wb_sel_reg <= 0;
    vector_wb_sel_reg <= 0;
    pc_sel_reg <= 0;
    scalar_pipeline_we_reg <= 0;
    vector_pipeline_we_reg <= 0;
    buffer_register_sel_reg <= 0;
    buffer_vector_sel_reg <= 0;
    buffer_register_reg <= 0;
    buffer_vector_reg <= 0;
  end else begin
    scalar_pipeline_wb_reg <= scalar_pipeline_wb;
    vector_pipeline_wb_reg <= vector_pipeline_wb;
    pc_reg <= pc;
    scalar_pipeline_vwb_reg <= scalar_pipeline_vwb;
    vector_pipeline_vwb_reg <= vector_pipeline_vwb;
    scalar_pipeline_mask_reg <= scalar_pipeline_mask;
    vector_pipeline_mask_reg <= vector_pipeline_mask;
    register_wb_sel_reg <= register_wb_sel;
    vector_wb_sel_reg <= vector_wb_sel;
    pc_sel_reg <= pc_sel_reg;
    scalar_pipeline_we_reg <= scalar_pipeline_we;
    vector_pipeline_we_reg <= vector_pipeline_we;
    buffer_register_sel_reg <= buffer_register_sel;
    buffer_vector_sel_reg <= buffer_vector_sel;
    buffer_register_reg <= buffer_register;
    buffer_vector_reg <= buffer_vector;
  end
end

endmodule
