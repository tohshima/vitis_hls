// AXIREG/UART command input task module for hackcpu
#ifndef __COMMAND_IN_TASK_HPP__
#define __COMMAND_IN_TASK_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "axireg_if.hpp"
#include "hackcpu.hpp"

void command_in_task(
	hls::stream<axireg_data_t>& axireg_command_in,
	hls::stream<token_word_t>& uart_in,
	hls::stream<command_t>& command_in,
	hls::stream<word_t>& ext_key_in,
	hls::stream<word_t>& ext_interrupt_in
);
#endif // #ifndef __COMMAND_IN_TASK_HPP__
