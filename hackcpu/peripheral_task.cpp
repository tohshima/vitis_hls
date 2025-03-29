// Peripheral task module for hackcpu
#include "hackcpu.hpp"
#include "peripheral_task.hpp"

// CPU peripheral module
//   Interrupt handling
//   Key input handling
//   Display out handling
#if 0
void interrupt_in_task(
	hls::stream<word_t>& ext_interrupt_in,
    hls::stream<word_t>& interrupt_in
) {
	#pragma HLS INTERFACE axis port=ext_interrupt_in depth=4
	#pragma HLS INTERFACE axis port=interrupt_in depth=4
    // Interrupt
    interrupt_in.write(ext_interrupt_in.read()); // ToDo: various interrupt should be handled in future
}
void peripheral_write_task(
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out
) {
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=128
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=128
	#pragma HLS INTERFACE axis port=dispadr_out depth=128
	#pragma HLS INTERFACE axis port=dispdat_out depth=128
    // Write access to peripheral from the cpu
    addr_t waddr = peripheral_waddr_out.read();
    word_t wdata = peripheral_wdata_out.read();
    if ((waddr >= PERIPHERAL_DISPMEM_ADDR) && (waddr < (PERIPHERAL_DISPMEM_ADDR+PERIPHERAL_DISPMEM_SIZE))) {
        // Display out
        dispadr_out.write(waddr);
        dispdat_out.write(wdata);
    }
}
void peripheral_read_task(
    hls::stream<word_t>& ext_key_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in
) {
	#pragma HLS INTERFACE axis port=ext_key_in depth=4
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=4
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=4
    // Read access to peripheral from the cpu
    static word_t key_code = 0;
    addr_t raddr = peripheral_raddr_out.read();
    if (raddr == PERIPHERAL_KEYIN_ADDR) {
        key_code = ext_key_in.read();
        peripheral_rdata_in.write(key_code);
    } else {
        // Send dummy data
        peripheral_rdata_in.write(0xDEAD);
    }
}
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
) {
	#pragma HLS INTERFACE axis port=ext_interrupt_in depth=1
	#pragma HLS INTERFACE axis port=interrupt_in depth=1
	#pragma HLS INTERFACE axis port=ext_key_in depth=2
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=1
	#pragma HLS INTERFACE axis port=dispadr_out depth=1
	#pragma HLS INTERFACE axis port=dispdat_out depth=1
	#pragma HLS INTERFACE axis port=dispflush_req depth=1
	#pragma HLS INTERFACE axis port=dispflush_ack depth=1

    // Interrupt or key_in
    if (!ext_interrupt_in.empty()) {
        interrupt_in.write(ext_interrupt_in.read()); // ToDo: various interrupt should be handled in future
    }
    // Key-in
    if (!ext_key_in.empty()) {
        word_t keyin = ext_key_in.read();
        interrupt_in.write(INT_REASON_KEYIN | (keyin & 0xFF));
    } 
    // Write access to peripheral from the cpu
    if (!peripheral_waddr_out.empty()) {
        addr_t waddr = peripheral_waddr_out.read();
        word_t wdata = peripheral_wdata_out.read();
        if ((waddr >= PERIPHERAL_DISPMEM_ADDR) && (waddr < (PERIPHERAL_DISPMEM_ADDR+PERIPHERAL_DISPMEM_SIZE))) {
            // Display out
            dispadr_out.write(waddr);
            dispdat_out.write(wdata);
        }
    }
}
#endif
