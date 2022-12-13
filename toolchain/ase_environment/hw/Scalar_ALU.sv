
module alu (A, B, op, out, zero, sign, overflow, clk, rst_n);
input clk, rst_n;
input [35:0] A, B;

input [3:0] op;  

output logic [35:0] out;

output logic zero, sign, overflow;  

logic next_zero, next_sign, next_overflow;

always_comb begin 
    case(op) 
        4'b0000: out = A + B;
        4'b0001: out = A - B;
        4'b0010: out = A * B;
        4'b0011: out = A & B;
        4'b0100: out = A | B;
        4'b0101: out = A ^ B;
        4'b0110: out = A << B[3:0];
        4'b0111: out = A >> B[3:0];
        4'b1000: out = A - B;
        default: out = 32'b0;
    endcase
end

assign next_zero = out != 36'b0;

assign next_sign = out[35];

assign next_overflow = ((A>0) & (B<0) & (out<0)) | ((A<0) & (B>0) & (out >0));



always_ff@(posedge clk, negedge !rst_n) begin
    if(!rst_n) begin
       zero <= '0;
       sign <= '0;
       overflow <= '0;
    end
    else if(op[3] == 1'b1) begin
       zero <= next_zero;
       sign <= next_sign;
       overflow <= next_overflow;
    end

end


endmodule