// HACKCPU UART IF top module 
#ifndef __HACKCPU_UART_HPP__
#define __HACKCPU_UART_HPP__

#include <ap_int.h>
#include "hackcpu.hpp"

int hackcpu_uart(
    #ifdef USE_ZYNQ_PS_UART
    ap_uint<1> start,
    #endif
    #ifdef USE_PYNQ_BUTTON
    volatile ap_uint<1> button_in0,
    volatile ap_uint<1> button_in1,
    volatile ap_uint<1> button_in2,
    volatile ap_uint<1> button_in3,
    volatile ap_uint<1> btn_smp_clk,
    volatile ap_uint<1>& led_btn_L_out,
    volatile ap_uint<1>& led_btn_R_out,
    volatile ap_uint<1>& led_active_out,
    #endif
	volatile unsigned int *uart_reg,
    volatile ap_uint<8>& debug_phase
);
#endif // #ifndef __HACKCPU_UART_HPP__
