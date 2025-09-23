// HACKCPU starttasks module 
#ifndef __START_TASKS_HPP__
#define __START_TASKS_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "hackcpu.hpp"
#include "axireg_if.hpp"

void start_tasks(
	hls::stream< ap_uint<1> >& led_active,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
    hls::stream<axireg_ext_t>& reg_ext_in,
    hls::stream<axireg_ext_t>& reg_ext_out,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<axireg_data_t>& reg_command_in_read,
    hls::stream<axireg_data_t>& reg_command_out_write
);
#endif // #ifndef __START_TASKS_HPP__
