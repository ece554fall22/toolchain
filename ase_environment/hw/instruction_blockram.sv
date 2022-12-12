module instruction_blockram(clk1, clk2, rd_addr, wr_data, wr_addr, wr_en, data_out);

output[511:0] data_out;		// Cache lines are 512 bits, this output is the same fourth of the Cacheline in the 4 different ways
input [127:0] wr_data;		// wr_data is the fourth of the cacheline to be written
input [8:0] rd_addr;		// rd_addr is smaller than wr_addr since reads need to read from all 4 ways but writes only write to 1
input [10:0] wr_addr;           
input clk1, clk2, wr_en;	// two input clocks justs because this is supported for blockram on fpga, should be the same clock since
				// we only need 1R 1W support for the blockram to work.
reg [9:0] rd_addr_reg;		// read request is pipelined so that this synthesizes as blockram
reg [511:0] data_out;		// same as output data_out written like this because that's how it's done in the intel example
reg [127:0] mem [2047:0];	// memory array that holds data


always @(posedge clk1) begin
  if (wr_en)						// write port
    mem[wr_addr] <= wr_data;	
end

always @(posedge clk2) begin				// read port
  data_out[127:0] <= mem[{rd_addr_reg, 2'b00}];
  data_out[255:128] <= mem[{rd_addr_reg, 2'b01}];
  data_out[383:256] <= mem[{rd_addr_reg, 2'b10}];
  data_out[511:384] <= mem[{rd_addr_reg, 2'b11}];
  rd_addr_reg <= rd_addr;				// rd request pipeline
end

endmodule