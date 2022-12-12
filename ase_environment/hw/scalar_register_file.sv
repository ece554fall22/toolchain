module scalar_register_file(read1, read2, write_addr, write_data, we, data1, data2, clk, rst_n);

input [35:0] write_data;
input [4:0] read1, read2, write_addr;
input clk, rst_n, we;
output logic [35:0] data1, data2;

logic [35:0] reg_data1, reg_data2;

ram_single r1 (.q(reg_data1), .a_in(write_addr), .a_out(read1), .d(write_data), .we(we), .clk1(clk), .clk2(~clk));
ram_single r2 (.q(reg_data2), .a_in(write_addr), .a_out(read2), .d(write_data), .we(we), .clk1(clk), .clk2(~clk));

assign data1 = (we & (read1 == write_addr)) ? write_data : reg_data1;
assign data2 = (we & (read2 == write_addr)) ? write_data : reg_data2;


endmodule
