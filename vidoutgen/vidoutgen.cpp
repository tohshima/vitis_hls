#include <ap_int.h>
#include <hls_stream.h>
#include <cstring>
#include "vidoutgen.hpp"
#include "vidoutgen_def.hpp"


static inline uint64_t combine_2pix(vidoutgen_rgba_t& col0, vidoutgen_rgba_t& col1) {
    return ((uint64_t)col1.a << 56) + ((uint64_t)col1.b << 48) + ((uint64_t)col1.g << 40) + ((uint64_t)col1.r << 32) +
           ((uint64_t)col0.a << 24) + ((uint64_t)col0.b << 16) + ((uint64_t)col0.g <<  8) + ((uint64_t)col0.r <<  0);
}

static inline void copy_line(uint64_t* p_dram, uint64_t* p_buf, uint64_t addr64, int repeat) {
    // バースト書き込み
    for (int i = 0; i < repeat; i++) {
    #pragma HLS PIPELINE II=1
        p_dram[addr64 + i] = p_buf[i];
    }     
}
static inline void make_hline(uint64_t* p_dram, uint64_t* p_buf, uint64_t addr64, uint16_t width, vidoutgen_rgba_t& col) {
    // width in unit of 2pix
    // まずバッファを初期化
    uint64_t data = combine_2pix(col, col);
    int repeat = width * sizeof(vidoutgen_rgba_t)/sizeof(uint64_t);
    for (int i = 0; i < repeat; i++) {
        #pragma HLS PIPELINE II=1
        p_buf[i] = data;
    }
    copy_line(p_dram, p_buf, addr64, repeat);   
}

#define BUF_SIZE (MAX_WIDTH*sizeof(vidoutgen_rgba_t)/sizeof(uint64_t))

void vidoutgen(
    vidoutgen_regs_t& regs,
    uint64_t* p_dram,
    hls::stream< hackcpu_video_t >& video_in_stream
) {
    #pragma HLS INTERFACE s_axilite port=regs register   
    #pragma HLS INTERFACE m_axi port=p_dram offset=direct \
        bundle=HP0 depth=BUF_SIZE \
        max_read_burst_length=MAX_BURST_NUM \
        max_write_burst_length=MAX_BURST_NUM \
        num_read_outstanding=MAX_BURST_NUM \
        num_write_outstanding=MAX_BURST_NUM \
        latency=64
    //#pragma HLS INTERFACE ap_none port=head_addr
    //#pragma HLS INTERFACE ap_none port=h

	#pragma HLS INTERFACE axis port=video_in_stream depth=16
	//#pragma HLS INTERFACE axis port=video_out_stream depth=MAX_BURST_NUM

    //#pragma HLS INTERFACE ap_none port=debug_vcounter    
    //#pragma HLS INTERFACE ap_none port=debug_data_in    
    //#pragma HLS INTERFACE ap_none port=debug_data_out    

    //#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_ctrl_hs port=return 

    static bool     enable = false;
    static uint16_t vcounter = 0;
    //static uint16_t bg_w = 0;
    //static uint16_t bg_h = 0;
    static bool  data_in = false;
    static bool data_out = false;

    uint64_t buffer[BUF_SIZE];  // 1280*4/8 = 640
    #pragma HLS ARRAY_PARTITION variable=buffer cyclic factor=16

    #define GET_INCR_SIZE(h) ((h*sizeof(vidoutgen_rgba_t))/sizeof(uint64_t))

    if (vidoutgen_get_control_enable(&regs)) {
        if (vidoutgen_get_control_cls(&regs)) {
            vidoutgen_rgba_t bg_col;
            vidoutgen_get_bg_color_as_rgba(&regs, &bg_col);
            vidoutgen_rgba_t fg_col0;
            vidoutgen_get_fg_color0(&regs, &fg_col0);
            uint16_t bgw  = vidoutgen_get_bg_width(&regs);
            uint64_t addr64 = vidoutgen_get_buf0_offset_addr(&regs)/sizeof(uint64_t);
            uint64_t addr64_incr = GET_INCR_SIZE(bgw);
            // top: bg
            uint16_t v0 = vidoutgen_get_fg_offset_y(&regs); 
            for (uint16_t v = 0; v < v0; v++) {
                make_hline(p_dram, buffer, addr64, bgw, bg_col);
                addr64 += addr64_incr;
            }
            // middle: bg fg bg
            uint16_t v1 = vidoutgen_get_fg_height(&regs);
            uint16_t bgw1_l = vidoutgen_get_fg_offset_x(&regs);
            uint64_t bgw1_l_incr64 = GET_INCR_SIZE(bgw1_l);
            uint16_t fgw = vidoutgen_get_fg_width(&regs);
            uint64_t fg_incr64 = GET_INCR_SIZE(fgw);
            uint16_t bgw1_r = bgw - (bgw1_l + fgw);
            uint64_t bgw1_r_incr64 = GET_INCR_SIZE(bgw1_r);
            for (uint16_t v = 0; v < v1; v++) {
                make_hline(p_dram, buffer, addr64, bgw1_l, bg_col);
                addr64 += bgw1_l_incr64;
                make_hline(p_dram, buffer, addr64, fgw, fg_col0);
                addr64 += fg_incr64;
                make_hline(p_dram, buffer, addr64, bgw1_r, bg_col);
                addr64 += bgw1_r_incr64;
            }
            // bottom: bg
            uint16_t v2 = vidoutgen_get_bg_height(&regs) - (v0 + v1); 
            for (uint16_t v = 0; v < v2; v++) {
                make_hline(p_dram, buffer, addr64, bgw, bg_col);
                addr64 += addr64_incr;
            }
            vidoutgen_clear_control_cls(&regs);
        } else {
            if (!video_in_stream.empty()) {
                vidoutgen_rgba_t fgc0, fgc1;
                vidoutgen_get_fg_color0(&regs, &fgc0);
                vidoutgen_get_fg_color1(&regs, &fgc1);
                const uint32_t fgc0_i = vidoutgen_conv_color_to_uint(&fgc0);
                const uint32_t fgc1_i = vidoutgen_conv_color_to_uint(&fgc1);

                const hackcpu_video_t vd = video_in_stream.read();
                const uint16_t bg_w = vidoutgen_get_bg_width(&regs);
                const uint16_t fg_offset_x = vidoutgen_get_fg_offset_x(&regs);
                const uint16_t fg_offset_y = vidoutgen_get_fg_offset_y(&regs);
                const uint16_t fg_x = vd.addr*sizeof(vd.data)*8 & 0x1FF; // % 512;
                const uint16_t fg_y = vd.addr*sizeof(vd.data)*8 >> 9;    // / 512;
                const uint64_t addr64 = ((bg_w*(fg_offset_y + fg_y) + fg_offset_x + fg_x) * sizeof(vidoutgen_rgba_t) + 
                                            vidoutgen_get_buf0_offset_addr(&regs))/ sizeof(uint64_t);
                const uint16_t repeat = sizeof(vd.data)*8 *sizeof(vidoutgen_rgba_t)/sizeof(uint64_t);
                for (uint16_t i = 0; i < repeat; i++) {
                    uint32_t p0 = (vd.data >> (2*i)) & 0x1;
                    uint32_t p1 = (vd.data >> (2*i+1)) & 0x1;
                    buffer[i] = (uint64_t)(p0? fgc1_i: fgc0_i) + ((uint64_t)(p1? fgc1_i: fgc0_i) << 32);
                }
                copy_line(p_dram, buffer, addr64, repeat);
            }
        }
    }
}
