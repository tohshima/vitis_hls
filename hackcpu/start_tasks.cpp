// HACKCPU starttask module
#include <hls_task.h>

#include "hackcpu.hpp"
#include "uart_if.hpp"
#include "command_in_task.hpp"
#include "comp_task.hpp"
#include "peripheral_task.hpp"
#include "command_out_task.hpp"
#include "axireg_if.hpp"
#include "start_tasks.hpp"

void start_tasks(
	hls::stream< ap_uint<1> >& led_active,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
    hls::stream<axireg_ext_t>& reg_ext_in,
    hls::stream<axireg_ext_t>& reg_ext_out,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<axireg_data_t>& reg_command_in_read,
    hls::stream<axireg_data_t>& reg_command_out_write
) {
    #pragma HLS INLINE

    #pragma HLS INTERFACE axis port=led_active depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_uart_enable_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_command_in_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_command_out_write depth=1

	// CPU interface signals
    hls_thread_local hls::stream<command_t> command_in;
    #pragma HLS STREAM variable=command_in depth=32
    hls_thread_local hls::stream<command_t> command_out;
    #pragma HLS STREAM variable=command_out depth=32
    hls_thread_local hls::stream<word_t> ext_key_in;
    #pragma HLS STREAM variable=ext_key_in depth=2
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
    #pragma HLS STREAM variable=dispadr_out depth=1
    hls_thread_local hls::stream<word_t> dispdat_out;
    #pragma HLS STREAM variable=dispdat_out depth=1
    hls_thread_local  hls::stream<bool> reg_uart_disp_enable_read;
    #pragma HLS STREAM variable=reg_uart_disp_enable_read depth=1
    hls_thread_local hls::stream<ap_uint<1> > dispflush_req;
    #pragma HLS STREAM variable=dispflush_req depth=1
    hls_thread_local hls::stream<ap_uint<1> > dispflush_ack;
    #pragma HLS STREAM variable=dispflush_ack depth=1

	hls_thread_local hls::task cit(command_in_task, reg_command_in_read, uart_in, command_in, ext_key_in, ext_interrupt_in);
	hls_thread_local hls::task ct(comp_task, 
        #ifdef USE_PYNQ_BUTTON
        led_active,
        #endif
        command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
	hls_thread_local hls::task pt(peripheral_task, ext_interrupt_in, interrupt_in, ext_key_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out, dispflush_req, dispflush_ack);
    //hls_thread_local hls::task itt(interrupt_in_task, ext_interrupt_in, interrupt_in);
    //hls_thread_local hls::task pwt(peripheral_write_task, peripheral_waddr_out, peripheral_wdata_out, dispadr_out, dispdat_out);
    //hls_thread_local hls::task prt(peripheral_read_task, ext_key_in, peripheral_raddr_out, peripheral_rdata_in);
	hls_thread_local hls::task cot(command_out_task, command_out, dispadr_out, dispdat_out, reg_command_out_write, uart_out, reg_uart_disp_enable_read, dispflush_req, dispflush_ack);
	hls_thread_local hls::task reg(axireg_task, reg_ext_in, reg_ext_out, reg_uart_enable_read, reg_uart_disp_enable_read, reg_command_in_read, reg_command_out_write);
}
