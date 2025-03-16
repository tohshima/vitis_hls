// HACKCPU starttask module
#include "hackcpu.hpp"
#include "uart_if.hpp"
#include "uart_in_task.hpp"
#include "comp_task.hpp"
#include "peripheral_task.hpp"
#include "uart_out_task.hpp"
#include "start_tasks.hpp"

void start_tasks(
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out
) {
    #pragma HLS INLINE

	// CPU interface signals
    hls_thread_local hls::stream<word_t> command_in;
    #pragma HLS STREAM variable=command_in depth=32
    hls_thread_local hls::stream<word_t> command_out;
    #pragma HLS STREAM variable=command_out depth=32
    hls_thread_local hls::stream<word_t> ext_key_in;
    #pragma HLS STREAM variable=ext_key_in depth=4
    //hls_thread_local hls::stream<word_t> key_in;
    //#pragma HLS STREAM variable=key_in depth=4
    hls_thread_local hls::stream<word_t> ext_interrupt_in;
    #pragma HLS STREAM variable=ext_interrupt_in depth=4
    hls_thread_local hls::stream<word_t> interrupt_in;
    #pragma HLS STREAM variable=interrupt_in depth=4
    hls_thread_local hls::stream<addr_t> peripheral_raddr_out;
    #pragma HLS STREAM variable=peripheral_raddr_out depth=1
    hls_thread_local hls::stream<word_t> peripheral_rdata_in;
    #pragma HLS STREAM variable=peripheral_rdata_in depth=1
    hls_thread_local hls::stream<addr_t> peripheral_waddr_out;
    #pragma HLS STREAM variable=peripheral_waddr_out depth=1
    hls_thread_local hls::stream<word_t> peripheral_wdata_out;
    #pragma HLS STREAM variable=peripheral_wdata_out depth=1
    hls_thread_local hls::stream<addr_t> dispadr_out;
    #pragma HLS STREAM variable=dispadr_out depth=128
    hls_thread_local hls::stream<word_t> dispdat_out;
    #pragma HLS STREAM variable=dispdat_out depth=128
    hls_thread_local hls::stream<ap_uint<1> > dispflush_req;
    #pragma HLS STREAM variable=dispflush_req depth=1
    hls_thread_local hls::stream<ap_uint<1> > dispflush_ack;
    #pragma HLS STREAM variable=dispflush_ack depth=1

	hls_thread_local hls::task uit(uart_in_task, uart_in, command_in, ext_key_in, ext_interrupt_in);
	hls_thread_local hls::task ct(comp_task, command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
	hls_thread_local hls::task pt(peripheral_task, ext_interrupt_in, interrupt_in, ext_key_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out, dispflush_req, dispflush_ack);
    //hls_thread_local hls::task itt(interrupt_in_task, ext_interrupt_in, interrupt_in);
    //hls_thread_local hls::task pwt(peripheral_write_task, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out);
    //hls_thread_local hls::task prt(peripheral_read_task, ext_key_in, peripheral_raddr_out, peripheral_rdata_in);
	hls_thread_local hls::task uot(uart_out_task, command_out, dispadr_out, dispdat_out, uart_out, dispflush_req, dispflush_ack);
}
