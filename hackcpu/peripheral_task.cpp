// Peripheral task module for hackcpu
#include "hackcpu.hpp"
#include "peripheral_task.hpp"

// CPU peripheral module
//   Interrupt handling
//   Key input handling
//   Display out handling
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
) {
	#pragma HLS INTERFACE axis port=ext_interrupt_in depth=4
	#pragma HLS INTERFACE axis port=interrupt_in depth=4
	#pragma HLS INTERFACE axis port=key_in depth=4
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=4
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=4
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=128
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=128
	#pragma HLS INTERFACE axis port=dispadr_out depth=128
	#pragma HLS INTERFACE axis port=dispdat_out depth=128

	// DISPLAY_MEMORY
	static word_t disp_ram[PERIPHERAL_DISPMEM_SIZE];
	#pragma HLS BIND_STORAGE variable=disp_ram type=RAM_2P impl=BRAM

    if (!ext_interrupt_in.empty()) {
        // Interrupt
        interrupt_in.write(ext_interrupt_in.read()); // ToDo: various interrupt should be handled in future
    } else if (!peripheral_waddr_out.empty()) {
        // Write access to peripheral from the cpu
        addr_t waddr = peripheral_waddr_out.read();
        word_t wdata = peripheral_wdata_out.read();
        if ((waddr >= PERIPHERAL_DISPMEM_ADDR) && (waddr < (PERIPHERAL_DISPMEM_ADDR+PERIPHERAL_DISPMEM_SIZE))) {
            // Display out
            dispadr_out.write(waddr);
            dispdat_out.write(wdata);
            disp_ram[waddr-PERIPHERAL_DISPMEM_ADDR] = wdata; // store for RMW
        }
    } else if (!peripheral_raddr_out.empty()) {
        // Read access to peripheral from the cpu
        addr_t raddr = peripheral_raddr_out.read();
        if (raddr == PERIPHERAL_KEYIN_ADDR) {
            word_t key_code = 0;
            if (!key_in.empty()) {
                key_code = key_in.read();
            }
            peripheral_rdata_in.write(key_code);
        } else if ((raddr >= PERIPHERAL_DISPMEM_ADDR) && (raddr < (PERIPHERAL_DISPMEM_ADDR+PERIPHERAL_DISPMEM_SIZE))) {
            // DISP MEM read
            peripheral_rdata_in.write(disp_ram[raddr-PERIPHERAL_DISPMEM_ADDR]);
        } else {
            // Send dummy data
            peripheral_rdata_in.write(0xDEAD);
        }
    }
}
