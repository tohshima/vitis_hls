#include <ap_int.h>
#include <hls_stream.h>
#include <cstring>
#include "vidoutgen.hpp"

static uint64_t read_buf[MAX_BURST_NUM];

void vidoutgen(
    
    uint64_t* p_dram,
    uint64_t head_addr,
    uint32_t h
) {
    //#pragma HLS INTERFACE s_axilite port=regs register   
#if 1
    #pragma HLS INTERFACE m_axi port=p_dram offset=direct \
        bundle=HP0 depth=1280*720*4/8 \
        max_read_burst_length=MAX_BURST_NUM \
        max_write_burst_length=MAX_BURST_NUM \
        num_read_outstanding=MAX_BURST_NUM \
        num_write_outstanding=MAX_BURST_NUM \
        latency=64
    #pragma HLS INTERFACE ap_none port=head_addr
    #pragma HLS INTERFACE ap_none port=h
#endif
	//#pragma HLS INTERFACE axis port=video_in_stream depth=16
	//#pragma HLS INTERFACE axis port=video_out_stream depth=MAX_BURST_NUM

    //#pragma HLS INTERFACE ap_none port=debug_vcounter    
    //#pragma HLS INTERFACE ap_none port=debug_data_in    
    //#pragma HLS INTERFACE ap_none port=debug_data_out    

    //#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_ctrl_hs port=return 

    static bool     enable = false;
    static uint16_t vcounter = 0;
    static uint16_t bg_w = 0;
    static uint16_t bg_h = 0;
    static bool  data_in = false;
    static bool data_out = false;

    #define BUF_SIZE (1280*4/sizeof(uint64_t))
    uint64_t buffer[BUF_SIZE];  // 1280*4/8 = 640
    #pragma HLS ARRAY_PARTITION variable=buffer cyclic factor=16

    uint32_t addr =(head_addr + 1280*4*h)/sizeof(uint64_t) ;
    // まずバッファを初期化
    for (int i = 0; i < BUF_SIZE; i++) {
        #pragma HLS PIPELINE II=1
        buffer[i] = 0x004488CC00CC8844ull;
        //p_dram[addr+i] = 0x004488CC00CC8844ull;
    }
        
    // バースト書き込み
    for (int i = 0; i < BUF_SIZE; i++) {
    #pragma HLS PIPELINE II=1
        p_dram[addr + i] = buffer[i];
    } 
    // update
    //debug_vcounter = vcounter;
    //debug_data_in = data_in? 1:0;
    //debug_data_out = data_out?  1:0;
}
