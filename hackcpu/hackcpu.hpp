#include <ap_int.h>
#include <cstdint>
#include <hls_stream.h>

// Define bit widths
const int WORD_WIDTH = 16;
const int ADDR_WIDTH = 15;

// Debug instrction
#define INST_FETCH_STOP 0x8000

// Define CPU components
typedef ap_uint<WORD_WIDTH> word_t;
typedef ap_uint<ADDR_WIDTH> addr_t;

// AXI interface for ROM
#include <ap_axi_sdata.h>
typedef ap_axiu<WORD_WIDTH, 0, 0, 0> axi_word_t;

// For debug
typedef struct {
    uint32_t    cycle;
    ap_uint<1>  write_out;
    word_t      outM;
    addr_t      addressM;
    addr_t      pc;
    word_t      regA;
    word_t      regD;
    word_t      alu_out;
    word_t      instruction;
} debug_s;

typedef hls::axis<debug_s, 0,0,0> debug_t;

void cpu(word_t rom[1 << ADDR_WIDTH],
         ap_uint<1>& reset, hls::stream<debug_t>& debug);
