module wb
( 
scalar_pipeline_wb, vector_pipeline_wb, pc, scalar_pipeline_vwb, vector_pipeline_vwb,
scalar_pipeline_we, vector_pipeline_we, pc_sel, scalar_pipeline_mask, vector_pipeline_mask,
register_wb_sel, vector_wb_sel, buffer_register_sel, buffer_vector_sel, buffer_register, buffer_vector,
vector_we, register_we, vector_data, register_data, clk, rst, scalar_pipeline_wbr,
vector_pipeline_wbr, vector_wbr, register_wbr
);
  input logic [35:0] scalar_pipeline_wb, vector_pipeline_wb, pc;	// scalar means data is from the scalar pipeline
  input logic [127:0] scalar_pipeline_vwb, vector_pipeline_vwb;		// wb means data for the register file
  input logic scalar_pipeline_we, vector_pipeline_we, pc_sel, clk, rst;		// vwb means data for the vector register file
  input logic [3:0] scalar_pipeline_mask, vector_pipeline_mask;		// we means write enable mask is for vector register
  input logic [4:0] scalar_pipeline_wbr, vector_pipeline_wbr;

// wb_sel from hazard detection unit chooses between scalar/vector pipelines
// outputs  that go to register files
  input register_wb_sel, vector_wb_sel;					

// used since vector pipeline outputs must be buffered
// if scalar pipeline wb wins since vector pipeline cannot stall
  input buffer_register_sel, buffer_vector_sel, buffer_register, buffer_vector; 
  output logic [3:0] vector_we;						
  output logic register_we;
  output logic [127:0] vector_data;
  output logic [35:0] register_data;
  output logic [4:0] vector_wbr;
  output logic [4:0] register_wbr;


// following signals are flopped and these are the outputs (used if one of the buffer signals go high)
  logic [35:0] vector_pipeline_wb_reg;
  logic [127:0] vector_pipeline_vwb_reg;
  logic vector_pipeline_we_reg;
  logic [3:0] vector_pipeline_mask_reg;
  logic [4:0] vector_pipeline_wbr_reg;

// makes vector we signal taking into account the mask
assign vector_we = (vector_wb_sel) ? 
		     ((buffer_vector_sel) ? 
                       vector_pipeline_mask_reg : 
                       vector_pipeline_mask) : 	
	             scalar_pipeline_mask;

// register write enable signal selected between vector_pipeline buffer vector_pipeline and scalar_pipeline versions
assign register_we = (register_wb_sel) ? 
		       ((buffer_register_sel) ? 
		         vector_pipeline_we_reg : 
		         vector_pipeline_we) : 
		        scalar_pipeline_we;	

// the vector write back register selected between values in vector and scalar pipelines
assign vector_wbr = (vector_wb_sel) ? 
		       ((buffer_vector_sel) ? 
		         vector_pipeline_wbr_reg : 
		         vector_pipeline_wbr) : 
		        scalar_pipeline_wbr;
					
// the vector write back register selected between values in vector and scalar pipelines
assign register_wbr = (register_wb_sel) ? 
		       ((buffer_register_sel) ? 
		         vector_pipeline_wbr_reg : 
		         vector_pipeline_wbr) : 
		        scalar_pipeline_wbr;
							
// same as prevous but for vector data		
assign vector_data = (vector_wb_sel) ? 					
                       ((buffer_vector_sel) ?  			       		
                         vector_pipeline_vwb_reg :
                         vector_pipeline_vwb) : 
                       scalar_pipeline_vwb;

// same as prevous but for register data and can also select pc from the scalar pipeline if pc_sel is high
assign register_data = (register_wb_sel) ? 
                         ((buffer_register_sel) ? 
                           vector_pipeline_wb_reg : 
                           vector_pipeline_wb) : 
                         ((~pc_sel) ?
		           scalar_pipeline_wb :
			   pc);

// buffers the register results of the vector pipeline
always_ff @(posedge clk, posedge rst) begin
  if(rst) begin
    vector_pipeline_wb_reg <= 0;
    vector_pipeline_we_reg <= 0;
    vector_pipeline_vwb_reg <= 0;
    vector_pipeline_wbr_reg <= 0;
    vector_pipeline_mask_reg <= 0;
  end else begin
    if (buffer_register) begin
    vector_pipeline_wb_reg <= vector_pipeline_wb;
    vector_pipeline_we_reg <= vector_pipeline_we;
    end
    if (buffer_vector) begin
    vector_pipeline_mask_reg <= vector_pipeline_mask;
    vector_pipeline_vwb_reg <= vector_pipeline_vwb;
    end
    if (buffer_register | buffer_vector)
    vector_pipeline_wbr_reg <= vector_pipeline_wbr;
  end
end

endmodule
