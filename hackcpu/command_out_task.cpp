// AXIREG/UART command out task module for hackcpu
#include "hackcpu.hpp"
#include "command_out_task.hpp"

static volatile char debug_phase_uot_ = 0;

static int make_hex_chars(
	word_t hex_data,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=128

	for (int i = 0; i < 4; i++) {
		char temp = ((hex_data >> (3 - i) * 4) & 0xF);
		char hex_char = (temp <= 9) ? temp + '0' : temp - 10 + 'a';
		uart_out.write(hex_char);
	}
	uart_out.write('\n');
	//hex_chars[5] = '\0';
	return 5;
}

static void send_four_chars(
	unsigned short data,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=128

	for (int i = 1; i <= 4; i++) {
		char temp = ((data >> (4 - i) * 4) & 0xF);
		char out = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
		uart_out.write(out);
	}
}

static int make_disp_out(
	word_t addrM,
	word_t dataM,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=4

	static word_t last_data_ = 0;
	static addr_t last_addr_ = 0;
    static bool first = true;
	unsigned short addr = addrM - 0x4000;
    if (first || (addr == 0)) {
        uart_out.write('!');
        send_four_chars(addr, uart_out);
        send_four_chars(dataM, uart_out);
        uart_out.write('\n');
        last_data_ = dataM;
        last_addr_ = addr;
        first = false;
        return 10;
    } else if (dataM == last_data_) {
		if ((addr-last_addr_) == 1) {
			// incremental address and the same data
			uart_out.write('&');
			uart_out.write('\n');
			last_addr_ = addr;
			return 2;
		} else if ((addr-last_addr_) == 0x20) {
			// incremental address in column direction and the same data
			uart_out.write('=');
			uart_out.write('\n');
			last_addr_ = addr;
			return 2;
		} else {
			// the same data
			uart_out.write('%');
			send_four_chars(addr, uart_out);
			uart_out.write('\n');
			last_addr_ = addr;
			return 6;
		}
	} else {
		if ((addr-last_addr_) == 1) {
			// incremental address
			uart_out.write('$');
			send_four_chars(dataM, uart_out);
			uart_out.write('\n');
			last_data_ = dataM;
			last_addr_ = addr;
			return 6;
		} else {
			uart_out.write('!');
			send_four_chars(addr, uart_out);
			send_four_chars(dataM, uart_out);
			uart_out.write('\n');
			last_data_ = dataM;
			last_addr_ = addr;
			return 10;
		}
	}
	return 0;
}

// convert command result output and display out data to uart chars for the controller GUI.
void command_out_task(
	hls::stream<command_t>& command_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out,
	hls::stream<axireg_data_t>& axireg_command_out,
	hls::stream<char>& uart_out,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<ap_uint<1> >& dispflush_req,
    hls::stream<ap_uint<1> >& dispflush_ack
) {
	#pragma HLS INTERFACE axis port=command_out depth=32
	#pragma HLS INTERFACE axis port=dispadr_out depth=1
	#pragma HLS INTERFACE axis port=dispdat_out depth=1
	#pragma HLS INTERFACE axis port=axireg_command_out depth=1
	#pragma HLS INTERFACE axis port=uart_out depth=1
	#pragma HLS INTERFACE axis port=reg_uart_disp_enable_read depth=1
	#pragma HLS INTERFACE axis port=dispflush_req depth=1
	#pragma HLS INTERFACE axis port=dispflush_ack depth=1

	if (!dispadr_out.empty()) {
		debug_phase_uot_ = 0xDD;
		word_t addrM = dispadr_out.read();
		word_t dataM = dispdat_out.read();
        if (reg_uart_disp_enable_read.read()) {
    		make_disp_out(addrM, dataM, uart_out);
        }
	} else if (!command_out.empty()) {
		debug_phase_uot_ = 0xD0;

        command_t cw0 = command_out.read();
		word_t num_ret = cw0.word;
        tag_t tag = cw0.tag;
        if (tag & CMDTAG_UART) {
    		make_hex_chars(num_ret, uart_out);
        } else {
            axireg_command_out.write(num_ret);
        }

		debug_phase_uot_ = 0xD2;
		for (int i = 0; i < num_ret; i++) {
            command_t cw1 = command_out.read();
            word_t data = cw1.word;
            if (tag & CMDTAG_UART) {
			    make_hex_chars(data, uart_out);
            } else {
                axireg_command_out.write(data);
            }
		}

		debug_phase_uot_ = 0xD3;
        command_t cw2 = command_out.read();
		word_t ret_status = cw2.word;
        if (tag & CMDTAG_UART) {
    		make_hex_chars(ret_status, uart_out);
        } else {
            axireg_command_out.write(ret_status);
        }
	} /*else if (!dispflush_req.empty()) {
        if (uart_out.size() == 0) { // all uart out completed.
            dispflush_req.read();
            dispflush_ack.write(1);
        }
    }*/
}
