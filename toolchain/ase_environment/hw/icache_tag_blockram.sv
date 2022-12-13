module icache_tag_blockram(tag_out, r_index, w_index, tag_in, wr_en, clk1, clk2);
  
  output[71:0] tag_out;		// tags are 18 bits and there are 4 tags per index so 72 bits
  input [17:0] tag_in;		// should only write 1 tag at a time so 18 bits
  input [6:0] r_index;		// r_index 2 less bits since you need to read all 4 tags of a given index
  input [8:0] w_index;
  input wr_en, clk1, clk2;	// 2 clocks should be the same in our design just here because thats how it is
				// in the intel example

  reg[6:0] r_index_reg;		// reg to infer blockram
  reg[71:0] tag_out;		// same as above written twice as in intel example
  reg[18:0] mem[1023:0];	// tag array

  always @(posedge clk1) begin				// wr port
    if(wr_en)
      mem[w_index] <= tag_in;
  end

  always @(posedge clk2) begin				// rd port
    tag_out[17:0] <= mem[{r_index_reg, 2'b00}];
    tag_out[35:18] <= mem[{r_index_reg, 2'b01}];
    tag_out[53:36] <= mem[{r_index_reg, 2'b10}];
    tag_out[71:54] <= mem[{r_index_reg, 2'b11}];
    r_index_reg <= r_index;				// rd request pipeline
  end


endmodule