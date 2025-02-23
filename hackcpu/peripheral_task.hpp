// Peripheral task module for hackcpu
#ifndef __PERIPHERAL_TASK_HPP__
#define __PERIPHERAL_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

void peripheral_task(
	hls::stream<word_t>& ext_interrupt_in,
    hls::stream<word_t>& interrupt_in,
    hls::stream<word_t>& key_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out
);
#endif // #ifndef __PERIPHERAL_TASK_HPP__
