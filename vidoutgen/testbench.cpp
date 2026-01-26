#include <cstdint>
#include <ap_int.h>
#include <hls_stream.h>
#include "vidoutgen.hpp"

static uint64_t dram[1280*720*4/8];

int main() {

    vidoutgen_regs_t regs;
    hls::stream< vidoutgen_rgb_t > video_out_stream;
    ap_uint<10> debug_vcounter;
    ap_uint<1> debug_data_in;
    ap_uint<1> debug_data_out;

    vidoutgen(
        //regs,
        dram,
        0x10,
        0
        //hls::stream< hackcpu_video_t >& video_in_stream,
        //video_out_stream//,
        //debug_vcounter,
        //debug_data_in,
        //debug_data_out
    );
    return 0;
}