#include <ap_int.h>
#include <hls_stream.h>

// Define bit widths
const int WORD_WIDTH = 16;
const int ADDR_WIDTH = 15;

// Define CPU components
typedef ap_uint<WORD_WIDTH> word_t;
typedef ap_uint<ADDR_WIDTH> addr_t;

// AXI interface for ROM
#include <ap_axi_sdata.h>
typedef ap_axiu<WORD_WIDTH, 0, 0, 0> axi_word_t;

void cpu(hls::stream<axi_word_t>& rom_in, hls::stream<axi_word_t>& rom_addr,
         ap_uint<1>& reset, ap_uint<1>& write_out, word_t& outM, addr_t& addressM, word_t& pc,
         word_t& debug_A, word_t& debug_D, word_t& debug_instruction);
