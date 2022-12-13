module metadata_registers(clk, rst, rd_addr, wr_addr, wr_en, data_in, data_out);
  output[10:0] data_out;	// metadata array {valid_array, dirty_array, plru}
  input [10:0] data_in;		// metadata array to be written
  input [7:0] rd_addr;		// index to read from
  input [7:0] wr_addr;		// index to write too
  input wr_en, clk, rst;

  reg [7:0] rd_addr_reg;	// pipelined rd_addr_reg
  reg [10:0] data_out;	
  reg [10:0] regs [255:0];	// register array	

  always @(posedge clk, posedge rst) begin
    if (rst) begin
      for(int i = 0; i < 256; i++) begin	// zeros array on reset
      regs[i] <= 0;
      end
    end else if (wr_en)
      regs[wr_addr] <= data_in;			// writes on wr_en and reads unconditionally
    data_out <= regs[rd_addr_reg];
    rd_addr_reg <= rd_addr;			// pipelined rd request
  end

endmodule