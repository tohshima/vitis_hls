// UART IF module for hackcpu
#ifndef __UART_IF_HPP__
#define __UART_IF_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void uart_if(
	volatile unsigned int *uart_reg,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
    hls::stream<bool>& reg_uart_enable_read,
    bool& sim_exit,
    volatile ap_uint<8>& debug_phase
);
#endif // #ifndef __UART_IF_HPP__
