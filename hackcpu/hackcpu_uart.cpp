// HACKCPU UART IF top module
#include <hls_task.h>

#include "hackcpu.hpp"
#include "start_tasks.hpp"
#include "uart_if.hpp"
#include "hackcpu_uart.hpp"

int hackcpu_uart(
	unsigned int *uart_reg
    #ifdef USE_ZYNQ_PS_UART
    ,
    bool start,
    #endif
) {
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
    #ifdef USE_ZYNQ_PS_UART
    #pragma HLS INTERFACE s_axillite register port=start
    #endif
    #pragma HLS INTERFACE ap_ctrl_none port=return

    #pragma HLS DATAFLOW
	hls_thread_local hls::stream<token_word_t> uart_in;
    #pragma HLS STREAM variable=uart_in depth=32
	hls_thread_local hls::stream<char> uart_out;
    #pragma HLS STREAM variable=uart_out depth=128
	
    start_tasks(uart_in, uart_out);
#ifndef SIM_TATSKS
    bool sim_exit = false;
	for(;;) {
        #pragma HLS PIPELINE
        #ifdef USE_ZYNQ_PS_UART
		uart_if(uart_reg, uart_in, uart_out, true, sim_exit);
        #else
		uart_if(uart_reg, uart_in, uart_out, sim_exit);
        #endif
        if (sim_exit) return 0;
	}
#endif
}
