// Computation task module for hackcpu
#include <hls_task.h>

#include "hackcpu.hpp"
#include "comp_task.hpp"

void peripheral_key_in_task(
    hls::stream<word_t>& ext_key_in,
    word_t d_ram[DRAM_SIZE]
) {
	#pragma HLS INTERFACE axis port=ext_key_in depth=4
    d_ram[PERIPHERAL_KEYIN_ADDR] = ext_key_in.read();
}
void comp_task(
	hls::stream<word_t>& command_in,
	hls::stream<word_t>& command_out,
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out
) {
    #pragma HLS INTERFACE axis port=command_in depth=32
    #pragma HLS INTERFACE axis port=command_out depth=32
	#pragma HLS INTERFACE axis port=interrupt_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=1

	cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
}

