module mem(clk, rst, line, w_type, cache_data, alu_data, mem_operation, register_wb, vector_wb);

// mem_operation is 1 if the current instruction uses the data cache output else it is 0
input clk, rst, mem_operation;
input [35:0] alu_data;
input [127:0] cache_data;

// w_type 00 means not a write, 01 means 32 bit write, 10 means 36 bit write, 11 means 128 bit write
input [1:0] line, w_type;

output [127:0] vector_wb;
output [35:0] register_wb;

logic [63:0] requested_line_64;
logic [31:0] requested_line;
logic [35:0] out_32, out_36, mem_wb;


// the following is a set of muxes that select the proper output datas to write back to the scalar and vector 
// register files
assign requested_line_64 = (line[1]) ? cache_data[127:64] : cache_data[63:0];
assign requested_line = (line[0]) ? requested_line_64[63:32] : requested_line_64[31:0];
assign out_32 = {4'h0, requested_line};
assign out_36 = requested_line_64[35:0];
assign vector_wb = cache_data;
assign mem_wb = (w_type[1]) ? out_36 : out_32;
assign register_wb = (mem_operation) ? mem_wb : alu_data;


endmodule
