// Copyright (c) 2020 University of Florida
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Greg Stitt
// University of Florida

// Module Name:  afu.sv
// Project:      dma_loopback
// Description:  This AFU provides a loopback DMA test that simply reads
//               data from one array in the CPU's memory and writes the
//               received data to a separate array. The AFU uses MMIO to
//               receive the starting read adress, starting write address,
//               size (# of cache lines to read/wite), and a go signal. The
//               AFU asserts a done signal to tell software that the DMA
//               transfer is complete.
//
//               One key difference with this AFU is that it does not use
//               CCI-P, which is abstracted away by a hardware abstraction
//               layer (HAL). Instead, the AFU uses a simplified MMIO interface
//               and DMA interface.
//
//               The MMIO interface is defined in mmio_if.vh. It behaves
//               similarly to the CCI-P functionality, except only supports
//               single-cycle MMIO read responses, which eliminates the need
//               for transaction IDs. MMIO writes behave identically to
//               CCI-P.
//
//               The DMA read interface takes a starting read address (rd_addr),
//               and a read size (rd_size) (# of cache lines to read). The rd_go
//               signal starts the transfer. When data is available from memory
//               the empty signal is cleared (0 == data available) and the data
//               is shown on the rd_data port. To read the data, the AFU should
//               assert the read enable (rd_en) (active high) for one cycle.
//               The rd_done signal is continuously asserted (active high) after
//               the AFU reads "size" words from the DMA.
//
//               The DMA write interface is similar, again using a starting
//               write address (wr_addr), write size (wr_size), and go signal.
//               Before writing data, the AFU must ensure that the write
//               interface is not full (full == 0). To write data, the AFU
//               puts the corresponding data on wr_data and asserts wr_en
//               (active high) for one cycle. The wr_done signal is continuosly
//               asserted after size cache lines have been written to memory.
//
//               All addresses are virtual addresses provided by the software.
//               All data elements are cachelines.
//

//===================================================================
// Interface Description
// clk  : Clock input
// rst  : Reset input (active high)
// mmio : Memory-mapped I/O interface. See mmio_if.vh and description above.
// dma  : DMA interface. See dma_if.vh and description above.
//===================================================================

`include "cci_mpf_if.vh"

module afu 
  (
   input clk,
   input rst,
	 mmio_if.user mmio,
	 dma_if.peripheral dma
   );

   localparam int CL_ADDR_WIDTH = $size(t_ccip_clAddr);
      
   // I want to just use dma.count_t, but apparently
   // either SV or Modelsim doesn't support that. Similarly, I can't
   // just do dma.SIZE_WIDTH without getting errors or warnings about
   // "constant expression cannot contain a hierarchical identifier" in
   // some tools. Declaring a function within the interface works just fine in
   // some tools, but in Quartus I get an error about too many ports in the
   // module instantiation.
   typedef logic [CL_ADDR_WIDTH:0] count_t;   
   logic 	go, reset, unhalt;
   logic 	done;

   // Software provides 64-bit virtual byte addresses.
   // Again, this constant would ideally get read from the DMA interface if
   // there was widespread tool support.
   localparam int VIRTUAL_BYTE_ADDR_WIDTH = 64;
   logic [VIRTUAL_BYTE_ADDR_WIDTH-1:0] start_addr;

   // Instantiate the memory map, which provides the starting read/write
   // 64-bit virtual byte addresses, a transfer size (in cache lines), and a
   // go signal. It also sends a done signal back to software.
   memory_map
     #(
       .ADDR_WIDTH(VIRTUAL_BYTE_ADDR_WIDTH),
       .SIZE_WIDTH(CL_ADDR_WIDTH+1)
       )
     memory_map (.*);

 logic done_in;
	

proc_hier proc_hier(.clk(clk), .rst(rst), .go(go), .done(done_in), .wr_offset(offset), .virt_addr_base(start_addr),
                    .wr_done(dma.wr_done), .rd_done(dma.rd_done), .full(dma.full), .empty(dma.empty),
                    .rd_data(dma.rd_data), .wr_data(dma.wr_data), .wr_addr(dma.wr_addr), .rd_addr(dma.rd_addr),
                    .rd_go(dma.rd_go), .wr_go(dma.wr_go), .rd_en(dma.rd_en), .wr_en(dma.wr_en), .host_rst(reset), 
                    .host_unhalt(unhalt));


   // Assign the starting addresses from the memory map.
   //assign dma.rd_addr = rd_addr;
   //assign dma.wr_addr = wr_addr;
   
   // Use the size (# of cache lines) specified by software.
   assign dma.rd_size = 16'h0001; // not using size anymore
   assign dma.wr_size = 16'h0001;

   // Start both the read and write channels when the MMIO go is received.
   // Note that writes don't actually occur until dma.wr_en is asserted.
   //assign dma.rd_go = go;
   //assign dma.wr_go = go;

   // Read from the DMA when there is data available (!dma.empty) and when
   // it is safe to write data (!dma.full).
   //assign dma.rd_en = !dma.empty && !dma.full;

   // Since this is a simple loopback, write to the DMA anytime we read.
   // For most applications, write enable would be asserted when there is an
   // output from a pipeline. In this case, the "pipeline" is a wire.
   //assign dma.wr_en = dma.rd_en;

   // Write the data that is read.
   //assign dma.wr_data = dma.rd_data;

   // The AFU is done when the DMA is done writing size cache lines.
   //assign done = // TODO add logic to make this
   always_ff @(posedge clk) begin
      if(rst) begin
         done <= 0;   
      end else if(done_in) begin
         done <= done_in;
      end
   end
            
endmodule
