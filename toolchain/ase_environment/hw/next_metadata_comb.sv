module next_metadata_comb(way, plru, valid_array, dirty_array, 
                   w_tagcheck, flushtype, hit, next_metadata);
  input [1:0] way, flushtype;		// way is selected victim in case of a miss or in the case of hit the way of the desired block
					// flushtype 11 = flushclean, 10 = flushdirty, 01 = flushline, 00 = noflush
  input [2:0] plru;			// a binary tree that points to the newer half of the 4 ways at each step
					// first bit points between ways 00/01 and ways 10/11 second bit is to
					// select between 11 and 10 third bit is to select between 10 and 00
  input [3:0] valid_array, dirty_array; // valid_array is the valid status of each way at the last read index
					// dirty_array is the same but for dirty bits.
  input w_tagcheck, hit;		// w_tagcheck means that the current read is being done to enable a write
					// hit is 1 if a valid tag match is found in the cache
  output [10:0] next_metadata;		// next_metadata is an array containing the next values of 
					// the dirty and valid arrays, and the plru bits

  logic [2:0] next_plru;		
  logic [3:0] next_valid_array, next_dirty_array, way_onehot;
					// way_onehot is just a onehot encoding of the 2 bit way signal
  
  // encodes the onehot form of way
  assign way_onehot = {&way, way[1] & ~way[0], ~way[1] & way[0], ~|way};

  assign next_plru[2] = way[1];			
  assign next_plru[1] = (way[1]) ? way[0] : plru[1];	// logic decides next plru based on which way is used
  assign next_plru[0] = (way[1]) ? plru[0] : way[0];

	// decides the next dirty array
  assign next_dirty_array = (hit) ? dirty_array | (way_onehot & {4{w_tagcheck}}) :
                          dirty_array & (~way_onehot | {4{|flushtype}});

always_comb begin
  case(flushtype)
    2'b11 : next_valid_array = valid_array & ~dirty_array;
    2'b10 : next_valid_array = valid_array & dirty_array;		// decides the next valid array
    2'b01 : next_valid_array = valid_array & (way_onehot & {4{hit}});
    2'b00 : next_valid_array = (hit) ? valid_array : valid_array | way_onehot; // make that way valid
  endcase
end
	// concatenate dirty, valid and plru arrays
  assign next_metadata = {next_valid_array, next_dirty_array, next_plru};

endmodule
