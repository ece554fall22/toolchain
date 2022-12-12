
module proc_hier(
input logic clk, rst,
input logic go,
output logic done,
input logic [35:0] wr_offset,
input logic [63:0] virt_addr_base,
input logic wr_done, rd_done,
input logic full, empty,
input logic [511:0] rd_data,
input logic host_rst, host_unhalt, // TODO unhalt should connect to proc, host_rst should just be an alternative rst idek if we need it// Copyright (c) 2020 University of Florida
output logic [511:0] wr_data,
output logic [63:0] wr_addr, rd_addr,
output logic rd_go, wr_go, rd_en, wr_en
);
logic [2:0] [2:0] packet_type_req_in, packet_type_req_out, packet_type_circ;
logic [2:0] [3:0] id_req_in, id_req_out, id_circ;
logic [2:0] [511:0] data_req_in, data_req_out, data_circ;
logic [2:0] [35:0] addr_req_in, addr_req_out, addr_circ;
logic r, mem_stall_in, instr_stall_in;
logic [2:0] w_type, overwrite, flushtype;
logic [35:0] addr;
logic [127:0] w_data;
logic [511:0] data_out;

data_cache_controller DCACHECONTROLLLER(.clk(clk), .rst(rst), .r(r), .faa(1'b0), .w_type(w_type), .addr(addr), 
                                       .mem_addr_in(addr_req_out[0]), .w_data(w_data), .mem_data_in(data_req_out[0]),
			               .id_req_in(id_req_out[0]), .packet_type_req_in(packet_type_req_out[0]),
			               .data_out(data_out), .mem_data_out(data_req_in[0]), .stall(mem_stall_in),
			               .overwrite(overwrite[0]), .mem_addr_out(addr_req_in[0]), .flushtype(flushtype),
			               .id_req_out(id_req_in[0]), .packet_type_req_out(packet_type_req_in[0]), .done(done));

// CMU0 connects to Dcache
circular_memory_unit CMU0(.clk(clk), .rst(rst), .addr_req_in(addr_req_in[0]), .addr_req_out(addr_req_out[0]),
                          .addr_circ_in(addr_circ[2]), .addr_circ_out(addr_circ[0]), .data_req_in(data_req_in[0]),
		          .data_req_out(data_req_out[0]), .data_circ_in(data_circ[2]), .data_circ_out(data_circ[0]),
		          .id_req_in(id_req_in[0]), .id_req_out(id_req_out[0]), .id_circ_in(id_circ[2]),
		          .id_circ_out(id_circ[0]), .packet_type_req_in(packet_type_req_in[0]),
		          .packet_type_req_out(packet_type_req_out[0]), .packet_type_circ_in(packet_type_circ[2]),
		          .packet_type_circ_out(packet_type_circ[0]), .overwrite(overwrite[0]));


icache_controller ICACHECONTROLLER (.clk(clk), .rst(rst), .r(1'b1), .faa(1'b0), .w_type(TODO), .flushtype(TODO),
                            .addr(TODO), .mem_addr_in(addr_req_out[1]), .w_data(TODO),.mem_data_in(data_req_out[1]),
                    .id_req_in(id_req_out[1]), .packet_type_req_in(packet_type_req_out[1]), .data_out(TODO),
                    .mem_data_out(data_req_in[1]), .stall(instr_stall_in), .overwrite(overwrite[1]), .mem_addr_out(addr_req_in[1]),
                    .id_req_out(id_req_in[1]), .packet_type_req_out(packet_type_req_in[1]));

// CMU1 connects to Icache
circular_memory_unit CMU1(.clk(clk), .rst(rst), .addr_req_in(addr_req_in[1]), .addr_req_out(addr_req_out[1]),
                          .addr_circ_in(addr_circ[0]), .addr_circ_out(addr_circ[1]), .data_req_in(data_req_in[1]),
		          .data_req_out(data_req_out[1]), .data_circ_in(data_circ[0]), .data_circ_out(data_circ[1]),
		          .id_req_in(id_req_in[1]), .id_req_out(id_req_out[1]), .id_circ_in(id_circ[0]),
		          .id_circ_out(id_circ[1]), .packet_type_req_in(packet_type_req_in[1]),
		          .packet_type_req_out(packet_type_req_out[1]), .packet_type_circ_in(packet_type_circ[0]),
		          .packet_type_circ_out(packet_type_circ[1]), .overwrite(overwrite[1]));


mem_controller MEM(.empty(empty), .rd_done(rd_done), .full(full), .wr_done(wr_done), .clk(clk), .rst(rst),
                   .rd_go(rd_go), .rd_en(rd_en), .wr_go(wr_go), .wr_en(wr_en),
	           .overwrite(overwrite[2]), .addr_in(addr_req_out[2]), .mmio_addr(virt_addr_offset), .rd_addr(rd_addr), 
	           .wr_addr(wr_addr), .addr_out(addr_out), .wr_size(wr_size), .cache_lines(cache_lines), .rd_data(rd_data),
	           .wr_data(wr_data), .data_in(data_req_out[2]), .data_out(data_req_in[2]), .id_req_in(id_req_out[2]), 
	           .id_req_out(id_req_in[2]), .packet_type_req_in(packet_type_req_out[2]),
	           .packet_type_req_out(packet_type_req_in[2]));

// CMU2 connects to mem controller
circular_memory_unit CMU2(.clk(clk), .rst(rst), .addr_req_in(addr_req_in[2]), .addr_req_out(addr_req_out[2]),
                          .addr_circ_in(addr_circ[1]), .addr_circ_out(addr_circ[2]), .data_req_in(data_req_in[2]),
		          .data_req_out(data_req_out[2]), .data_circ_in(data_circ[1]), .data_circ_out(data_circ[2]),
		          .id_req_in(id_req_in[2]), .id_req_out(id_req_out[2]), .id_circ_in(id_circ[1]),
		          .id_circ_out(id_circ[2]), .packet_type_req_in(packet_type_req_in[2]),
		          .packet_type_req_out(packet_type_req_out[2]), .packet_type_circ_in(packet_type_circ[1]),
		          .packet_type_circ_out(packet_type_circ[2]), .overwrite(overwrite[2]));

proc PROC0();

endmodule
