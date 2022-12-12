module register_file(read1, read2, write_addr, write_data, we, data1, data2, clk, rst_n);

input [35:0] write_data;
input [4:0] read1, read2, write_addr;
input clk, rst_n, we;
output logic data1, data2;

logic reg_data1, reg_data2;

ram_single r1 (.q(write_data), .a_in(read1), .a_out(write_addr), .d(reg_data1), .we(we), .clk(clk));
ram_single r2 (.q(write_data), .a_in(read1), .a_out(write_addr), .d(reg_data2), .we(we), .clk(clk));

assign data1 = (we & (read1 == write_addr)) ? write_data : reg_data1;
assign data2 = (we & (read2 == write_addr)) ? write_data : reg_data2;


endmodule
