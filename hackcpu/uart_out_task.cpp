// UART input task module for hackcpu
#include "hackcpu.hpp"
#include "uart_in_task.hpp"

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
void uart_out_task(
	hls::stream<word_t>& command_out,
	hls::stream<addr_t>& dispadr_out,
	hls::stream<word_t>& dispdat_out,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=command_out depth=32
	#pragma HLS INTERFACE axis port=dispadr_out depth=4
	#pragma HLS INTERFACE axis port=dispdat_out depth=4
	#pragma HLS INTERFACE axis port=uart_out depth=4

	if (!command_out.empty()) {
		debug_phase_uot_ = 0xD0;

		word_t num_ret = command_out.read();
		make_hex_chars(num_ret, uart_out);

		debug_phase_uot_ = 0xD2;
		for (int i = 0; i < num_ret; i++) {
			word_t data = command_out.read();
			make_hex_chars(data, uart_out);
		}

		debug_phase_uot_ = 0xD3;
		word_t ret_status = command_out.read();
		make_hex_chars(ret_status, uart_out);
	} else if (!dispadr_out.empty()) {
		debug_phase_uot_ = 0xDD;
		word_t addrM = dispadr_out.read();
		word_t dataM = dispdat_out.read();
		make_disp_out(addrM, dataM, uart_out);
	}
}
