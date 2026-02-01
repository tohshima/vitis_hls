#ifndef __VIDOUTGEN_HPP__
#define __VIDOUTGEN_HPP__

#include <ap_int.h>
#include <hls_stream.h>

#include "vidoutgen_def.hpp"

void vidoutgen(
    vidoutgen_regs_t& regs,
    uint64_t* p_dram
    //hls::stream< hackcpu_video_t >& video_in_stream,
    //hls::stream< vidoutgen_rgb_t >& video_out_stream//,
    //ap_uint<10>& debug_vcounter,
    //ap_uint<1>& debug_data_in,
    //ap_uint<1>& debug_data_out
);
#endif // #ifndef __VIDOUTGEN_HPP__
