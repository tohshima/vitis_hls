// Peripheral task module for hackcpu
#ifndef __PERIPHERAL_TASK_HPP__
#define __PERIPHERAL_TASK_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"
#if 0
void interrupt_in_task(
	hls::stream<word_t>& ext_interrupt_in,
    hls::stream<word_t>& interrupt_in
);
void peripheral_write_task(
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out
) ;
void peripheral_read_task(
    hls::stream<word_t>& ext_key_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in
) ;
#else
void peripheral_task(
	hls::stream<word_t>& ext_interrupt_in,
    hls::stream<word_t>& interrupt_in,
    hls::stream<word_t>& ext_key_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out,
    hls::stream<ap_uint<1> >& dispflush_req,
    hls::stream<ap_uint<1> >& dispflush_ack
);
#endif
#endif // #ifndef __PERIPHERAL_TASK_HPP__
