// HACKCPU UART IF top module
#include "hackcpu.hpp"
#include "start_tasks.hpp"
#include "uart_if.hpp"
#include "hackcpu_uart.hpp"

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
	
    word_t peripheral_mem[PERIPHERAL_MEM_SIZE] = {0};
    #pragma HLS BIND_STORAGE variable=peripheral_mem ty@e = RAM_T2P

    start_tasks(peripheral_mem, uart_in, uart_out);
#ifndef SIM_TATSKS
    bool sim_exit = false;
	for(;;) {
        #pragma HLS PIPELINE
		uart_if(uart_reg, uart_in, uart_out, sim_exit);
        if (sim_exit) return 0;
	}
#endif
}
