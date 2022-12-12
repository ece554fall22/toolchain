module circular_memory_unit #(
parameter DEPTH = 512
)
(
input logic clk, rst, overwrite,
input logic [35:0] addr_circ_in, addr_req_in,
output logic [35:0] addr_circ_out, addr_req_out,
input logic [DEPTH-1:0] data_circ_in, data_req_in,
output logic [DEPTH-1:0] data_circ_out, data_req_out,
input logic [4:0] id_circ_in, id_req_in,
output logic [4:0] id_circ_out, id_req_out,
input logic [2:0] packet_type_circ_in, packet_type_req_in,
output logic [2:0] packet_type_circ_out, packet_type_req_out
);
logic [35:0] next_addr_circ_out;
logic [DEPTH-1:0] next_data_circ_out;
logic [4:0] next_id_circ_out;
logic [2:0] next_packet_type_circ_out;

// outputs to connected module are always circ inputs
assign addr_req_out = addr_circ_in;
assign data_req_out = data_circ_in;
assign id_req_out = id_circ_in;
assign packet_type_req_out = packet_type_circ_in;

// outputs to next circular unit can be overwritten by connected module
assign next_addr_circ_out = (overwrite) ? addr_req_in : addr_circ_in;
assign next_data_circ_out = (overwrite) ? data_req_in : data_circ_in;
assign next_id_circ_out = (overwrite) ? id_req_in : id_circ_in;
assign next_packet_type_circ_out = (overwrite) ? packet_type_req_in : packet_type_circ_in;

// flops for outputs
always_ff @(posedge clk, posedge rst) begin
  if(rst) begin
    addr_circ_out <= 0;
    data_circ_out <= 0;
    id_circ_out <= 0;
    packet_type_circ_out <= 0;
  end else if(rst) begin
    addr_circ_out <= next_addr_circ_out;
    data_circ_out <= next_data_circ_out;
    id_circ_out <= next_id_circ_out;
    packet_type_circ_out <= next_packet_type_circ_out;
  end
end

endmodule
