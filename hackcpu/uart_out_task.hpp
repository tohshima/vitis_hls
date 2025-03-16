// UART output task module for hackcpu
#ifndef __UART_OUT_TASK_HPP__
#define __UART_OUT_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void uart_out_task(
	hls::stream<word_t>& command_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out,
	hls::stream<char>& uart_out,
    hls::stream<ap_uint<1> >& dispflush_req,
    hls::stream<ap_uint<1> >& dispflush_ack
);
#endif // #ifndef __UART_OUT_TASK_HPP__
