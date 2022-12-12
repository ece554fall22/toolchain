module vector_register_file(read1, read2, write_addr, write_vector, data1, data2, we, clk, rst_n);

parameter VECTOR_REG_SIZE = 32;
parameter VECTOR_REG_WIDTH = 4;

input clk, rst_n;
input [31:0] write_vector [3:0];
input[4:0] read1, read2, write_addr;
input [3:0] we;
output logic [31:0] data1 [3:0];
output logic [31:0] data2 [3:0];

logic [31:0] reg_data1 [3:0];
logic [31:0] reg_data2 [3:0];

logic [31:0] out1 [3:0];
logic [31:0] out2 [3:0];



genvar i;
generate;
    for(i =0; i <4; i++) begin
        ram_single #(.REG_SIZE(32)) r1 (.q(write_vector[i]), .a_in(write_addr), .a_out(read1), .d(reg_data1[i]), .we(we[i]), .clk1(clk), .clk2(~clk));
        ram_single #(.REG_SIZE(32)) r2 (.q(write_vector[i]), .a_in(write_addr), .a_out(read2), .d(reg_data2[i]), .we(we[i]), .clk1(clk), .clk2(~clk));
    end
    for(i =0; i <4; i++) begin
        assign out1[i] = (we[i] & (read1 == write_addr)) ? write_vector[i] : reg_data1[i];
        assign out2[i] = (we[i] & (read2 == write_addr)) ? write_vector[i] : reg_data2[i];
    end
    for(i = 0; i <4; i++) begin
        assign data1[i] = out1[i];
        assign data2[i] = out2[i];
    end
endgenerate

endmodule