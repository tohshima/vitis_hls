#include <cstdint>
#include <ap_int.h>
#include <hls_stream.h>
#include "vidoutgen.hpp"

//static uint64_t dram[1280*720*4/8];
static uint64_t dram[32*4*4/8];

int main() {

    vidoutgen_regs_t regs;
    hls::stream< vidoutgen_rgb_t > video_out_stream;
    ap_uint<10> debug_vcounter;
    ap_uint<1> debug_data_in;
    ap_uint<1> debug_data_out;

    regs.bg_col_b = 0x11;
    regs.bg_col_g = 0x22;
    regs.bg_col_r = 0x33;
    regs.bg_height = 4;
    regs.bg_width = 16;
    regs.fg_height = 2;
    regs.fg_width = 8;
    regs.fg_offset_x = 4;
    regs.fg_offset_y = 1;
    regs.fg_col0_a = 0x00;
    regs.fg_col0_b = 0xAA;
    regs.fg_col0_g = 0xBB;
    regs.fg_col0_r = 0xCC;
    regs.control = 3;
    regs.buf0_addr_low = 0x0;
    regs.buf0_addr_high = 0;

    vidoutgen(
        regs,
        dram
        //hls::stream< hackcpu_video_t >& video_in_stream,
        //video_out_stream//,
        //debug_vcounter,
        //debug_data_in,
        //debug_data_out
    );
    return 0;
}