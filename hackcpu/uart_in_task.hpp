// UART input task module for hackcpu
#ifndef __UART_IN_TASK_HPP__
#define __UART_IN_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void uart_in_task(
	hls::stream<token_word_t>& uart_in,
	hls::stream<word_t>& command_in,
	hls::stream<word_t>& ext_key_in,
	hls::stream<word_t>& ext_interrupt_in
);
#endif // #ifndef __UART_IN_TASK_HPP__
