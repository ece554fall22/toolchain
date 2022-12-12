module synch_mem #(
parameter DEPTH = 512
)
(
input clk, rst, 
output overwrite,
input [35:0] addr_req_in,
output [35:0] addr_req_out,
input [DEPTH-1:0] data_in,
output [DEPTH-1:0] data_out,
input [3:0] id_req_in,
output [3:0] id_req_out,
input [2:0] packet_type_req_in,
output [2:0] packet_type_req_out
);

logic [3:0] [31:0] synch_regs;
logic [31:0] synch_regs_write;
logic [3:0] wr_ens;

  assign synch_regs_write = (packet_type_req_in==3'b111) ? data_in[31:0] : data_in[31:0] + {{17{data_in[46]}}, data_in[46:32]};

assign id_req_out = id_req_in;

assign packet_type_req_in = (packet_type_req_in==3'b010) ? 3'b101 : 3'b110;

assign overwrite = (packet_type_req_in==3'b111) | (packet_type_req_in==3'b010) | (packet_type_req_in==3'b100);

assign data_out = {{(DEPTH-32){1'b0}}, synch_regs[addr_req_in[3:2]]};

assign addr_req_out = addr_req_in;



genvar i;
generate
  for(i = 0; i<4; i++) begin
    always_ff @(posedge clk, posedge rst) begin
      if(rst) begin
        synch_regs[i] <= 0;
      end else if (wr_ens[i])begin
        synch_regs[i] <= synch_regs_write;
      end
    end
  end
endgenerate

endmodule
