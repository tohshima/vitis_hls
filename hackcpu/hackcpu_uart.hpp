// HACKCPU UART IF top module 
#ifndef __HACKCPU_UART_HPP__
#define __HACKCPU_UART_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void start_tasks(
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out
);
int hackcpu_uart(
	unsigned int *uart_reg
);
#endif // #ifndef __HACKCPU_UART_HPP__
