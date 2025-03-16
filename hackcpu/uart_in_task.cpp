// UART input task module for hackcpu
#include "hackcpu.hpp"
#include "uart_in_task.hpp"

static volatile char debug_phase_uit_ = 0;
static volatile word_t debug_command_ = 0;

static char convert2hex(char c) {
	char h = 0;
	if ((c >= '0') && (c <= '9')) { h = (c-'0'); }
	else if ((c >= 'a') && (c <= 'f')) { h = (c-'a'+10); }
	else if ((c >= 'A') && (c <= 'F')) { h = (c-'A'+10); }
	return h;
}
static word_t make_hex_bin(uint32_t hex_chars4) {
	word_t hex_data = 0;
	for (int i = 0; i < 4; i++) {
		char c = (hex_chars4 >> i*8) & 0xFF;
		hex_data += convert2hex(c) << (3-i)*4;
	}
	return hex_data;
}

// convert sequence of characters via uart to command words, key input or interrupt signal.
void uart_in_task(
	hls::stream<token_word_t>& uart_in,
	hls::stream<word_t>& command_in,
	hls::stream<word_t>& ext_key_in,
	hls::stream<word_t>& ext_interrupt_in
) {
	#pragma HLS INTERFACE axis port=uart_in depth=4
	#pragma HLS INTERFACE axis port=command_in depth=32
	#pragma HLS INTERFACE axis port=ext_key_in depth=4
	#pragma HLS INTERFACE axis port=ext_interrupt_in depth=4

	static word_t read_data[16];
	#pragma HLS BIND_STORAGE variable=read_data type=RAM_T2P impl=BRAM

	debug_phase_uit_ = 0xB0;
	//if (!uart_in.empty()) {
		token_word_t token = uart_in.read();
        token_word_t token0 = token & 0xFF;
		if (token0 == 'K') {
            // key input
			debug_phase_uit_ = 0xB1;
			word_t key_code = (((token >> 8)&0xFF)-'0')*100+(((token >> 16)&0xFF)-'0')*10+(((token >> 24)&0xFF)-'0'); // ToDo: conversion is not general
            ext_key_in.write_nb(key_code);
                //std::cerr << "Sent " << key_code << std::endl;
		} else if (token0 == 'I') {
            // interrupt
			debug_phase_uit_ = 0xB2;
            ext_interrupt_in.write(0x8000);
		} else {
            // commands
			debug_phase_uit_ = 0xB3;
			word_t command = make_hex_bin(token);
			debug_command_ = command;
			command_in.write(command);
		}
	//}
}

