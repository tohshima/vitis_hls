// HACKCPU starttasks module 
#ifndef __START_TASKS_HPP__
#define __START_TASKS_HPP__

#include <ap_int.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void start_tasks(
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out
);
#endif // #ifndef __START_TASKS_HPP__
