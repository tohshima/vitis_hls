// AXIREG/UART command output task module for hackcpu
#ifndef __COMMAND_OUT_TASK_HPP__
#define __COMMAND_OUT_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include "axireg_if.hpp"
#include "hackcpu.hpp"
#include "../vidoutgen/vidoutgen_def.hpp"

void command_out_task(
	hls::stream<command_t>& command_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out,
	hls::stream< hackcpu_video_t >& video_stream,
	hls::stream<hackcpu_reg_t>& axireg_command_out,
	hls::stream<char>& uart_out,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<ap_uint<1> >& dispflush_req,
    hls::stream<ap_uint<1> >& dispflush_ack
);
#endif // #ifndef __COMMAND_OUT_TASK_HPP__
