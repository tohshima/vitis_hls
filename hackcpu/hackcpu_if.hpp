// HACKCPU IF top module 
#ifndef __HACKCPU_IF_HPP__
#define __HACKCPU_IF_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "axireg_if.hpp"
#include "../vidoutgen/vidoutgen_def.hpp"
#include "hackcpu.hpp"

int hackcpu_if(
    hackcpu_regs_t& axi_regs,
	hls::stream< hackcpu_video_t >& video_stream,
    volatile ap_uint<1> button_in0,
    volatile ap_uint<1> button_in1,
    volatile ap_uint<1> button_in2,
    volatile ap_uint<1> button_in3,
    volatile ap_uint<1> btn_smp_clk,
    volatile ap_uint<1>& led_btn_L_out,
    volatile ap_uint<1>& led_btn_R_out,
    volatile ap_uint<1>& led_active_out,
	volatile unsigned int *uart_reg,
    volatile ap_uint<8>& debug_phase
);
#endif // #ifndef __HACKCPU_UART_HPP__
