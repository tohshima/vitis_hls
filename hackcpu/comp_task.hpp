// Computation task module for hackcpu
#ifndef __COMP_TASK_HPP__
#define __COMP_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void comp_task(
	hls::stream<word_t>& command_in,
	hls::stream<word_t>& command_out,
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out
);
#endif // #ifndef __COMP_TASK_HPP__
