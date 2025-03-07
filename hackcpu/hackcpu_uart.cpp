// HACKCPU UART IF top module
#include "hackcpu.hpp"
#include "uart_if.hpp"
#include "uart_in_task.hpp"
#include "comp_task.hpp"
#include "peripheral_task.hpp"
#include "uart_out_task.hpp"
#include "hackcpu_uart.hpp"

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
    hls_thread_local hls::stream<word_t> key_in;
    #pragma HLS STREAM variable=key_in depth=4
    hls_thread_local hls::stream<word_t> ext_interrupt_in;
    #pragma HLS STREAM variable=ext_interrupt_in depth=4
    hls_thread_local hls::stream<word_t> interrupt_in;
    #pragma HLS STREAM variable=interrupt_in depth=4
    hls_thread_local hls::stream<addr_t> peripheral_raddr_out;
    #pragma HLS STREAM variable=peripheral_raddr_out depth=4
    hls_thread_local hls::stream<word_t> peripheral_rdata_in;
    #pragma HLS STREAM variable=peripheral_rdata_in depth=4
    hls_thread_local hls::stream<addr_t> peripheral_waddr_out;
    #pragma HLS STREAM variable=peripheral_waddr_out depth=128
    hls_thread_local hls::stream<word_t> peripheral_wdata_out;
    #pragma HLS STREAM variable=peripheral_wdata_out depth=128
    hls_thread_local hls::stream<addr_t> dispadr_out;
    #pragma HLS STREAM variable=dispadr_out depth=128
    hls_thread_local hls::stream<word_t> dispdat_out;
    #pragma HLS STREAM variable=dispdat_out depth=128

	hls_thread_local hls::task uit(uart_in_task, uart_in, command_in, key_in, ext_interrupt_in);
	hls_thread_local hls::task ct(comp_task, command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
	hls_thread_local hls::task pt(peripheral_task, ext_interrupt_in, interrupt_in, key_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out);
	hls_thread_local hls::task uot(uart_out_task, command_out, dispadr_out, dispdat_out, uart_out);
}

#ifndef SIM_TASKS
int hackcpu_uart(
	unsigned int *uart_reg
) {
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
    #pragma HLS INTERFACE ap_none port=return

    #pragma HLS DATAFLOW
	hls_thread_local hls::stream<token_word_t> uart_in;
    #pragma HLS STREAM variable=uart_in depth=32
	hls_thread_local hls::stream<char> uart_out;
    #pragma HLS STREAM variable=uart_out depth=128
	
    start_tasks(uart_in, uart_out);

    bool sim_exit = false;
	for(;;) {
        #pragma HLS PIPELINE
		uart_if(uart_reg, uart_in, uart_out, sim_exit);
        if (sim_exit) return 0;
	}
}
#endif
