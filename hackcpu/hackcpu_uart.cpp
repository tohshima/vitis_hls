// HACKCPU UART IF top module
#include "hackcpu.hpp"
#include "uart_if.hpp"
#include "uart_in_task.hpp"
#include "comp_task.hpp"
#include "peripheral_task.hpp"
#include "uart_out_task.hpp"
#include "hackcpu_uart.hpp"

int hackcpu_uart(
	volatile unsigned int *uart_reg
) {
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない

	bool start = true;
	volatile char debug_phase__ = 0;
	volatile word_t debug_command__ = 0;
	volatile char debug_rx_data__ = 0;
	volatile char debug_injection = 0;

	static hls_thread_local hls::stream<token_word_t> uart_in;
	static hls_thread_local hls::stream<char> uart_out;
	// CPU interface signals
    static hls_thread_local hls::stream<word_t> command_in;
    static hls_thread_local hls::stream<word_t> command_out;
    static hls_thread_local hls::stream<word_t> key_in;
    static hls_thread_local hls::stream<word_t> ext_interrupt_in;
    static hls_thread_local hls::stream<word_t> interrupt_in;
    static hls_thread_local hls::stream<addr_t> peripheral_raddr_out;
    static hls_thread_local hls::stream<word_t> peripheral_rdata_in;
    static hls_thread_local hls::stream<addr_t> peripheral_waddr_out;
    static hls_thread_local hls::stream<word_t> peripheral_wdata_out;
    static hls_thread_local hls::stream<addr_t> dispadr_out;
    static hls_thread_local hls::stream<word_t> dispdat_out;


	hls_thread_local hls::task uit(uart_in_task, uart_in, command_in, key_in, ext_interrupt_in);
	hls_thread_local hls::task ct(comp_task, command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
	hls_thread_local hls::task pt(peripheral_task, ext_interrupt_in, interrupt_in, key_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out);
	hls_thread_local hls::task uot(uart_out_task, command_out, dispadr_out, dispdat_out, uart_out);

    volatile bool sim_exit = false;
	uart_if(start, uart_reg, uart_in, uart_out, sim_exit, debug_phase__, debug_rx_data__);
	while(1) {
		uart_if(start, uart_reg, uart_in, uart_out, sim_exit, debug_phase__, debug_rx_data__);
        if (sim_exit) return 0;
	}
}
