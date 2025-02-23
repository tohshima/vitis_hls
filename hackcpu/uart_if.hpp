// UART IF module for hackcpu
#ifndef __UART_IF_HPP__
#define __UART_IF_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void uart_if(
	bool start,
	volatile unsigned int *uart_reg,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
    volatile bool& sim_exit,
	volatile char& debug_phase__,
	volatile char& debug_rx_data__
);
#endif // #ifndef __UART_IF_HPP__
