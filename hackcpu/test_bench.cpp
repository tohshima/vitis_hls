#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_stream.h>
#include "../common/uart_comm.hpp"
#include "revasm.hpp"
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h

#define USE_COM "COM2"

#define DUMMY_READ() {while(!command_out.empty()) {command_out.read();}}

void read_rom_file(const std::string& filename,
             hls::stream<word_t>& command_in,
             hls::stream<word_t>& command_out) {
    std::ifstream file(filename);
    std::string line;

    word_t tmp[IRAM_SIZE];
    
    int length = 0;
    while (std::getline(file, line)) {
        word_t instruction = 0;
        for (char c : line) {
            instruction = (instruction << 1) | (c - '0');
        }
        tmp[length++] = instruction;
        printf("%s %08x\n", line.c_str(), instruction.to_ushort());
    }
    command_in.write(LOAD_TO_IRAM);
    command_in.write(0);
    command_in.write(length);
    for (int i = 0; i < length; i++) {
        command_in.write(tmp[i]);
    }
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
}

void get_debug_info(hls::stream<word_t>& command_in, hls::stream<word_t>& command_out, word_t bitmap, debug_s& dinfo) {
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(GET_DEBUG_INFO);
    command_in.write(bitmap);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
    
    if (bitmap & DINFO_BIT_CYCLE) {
        uint64_t cycle = 0;
        for (int i = 0; i < 4; i++) {
            cycle |= (uint64_t)command_out.read().to_ushort() << 16*i;
        }
        dinfo.cycle = cycle;
    }
    if (bitmap & DINFO_BIT_WOUT) dinfo.write_out = command_out.read();
    if (bitmap & DINFO_BIT_OUTM) dinfo.outM = command_out.read();
    if (bitmap & DINFO_BIT_ADDRM) dinfo.addressM = command_out.read();
    if (bitmap & DINFO_BIT_PC) dinfo.pc = command_out.read();
    if (bitmap & DINFO_BIT_REGA) dinfo.regA = command_out.read();
    if (bitmap & DINFO_BIT_REGD) dinfo.regD = command_out.read();
    if (bitmap & DINFO_BIT_ALUO) dinfo.alu_out = command_out.read();
    if (bitmap & DINFO_BIT_INST1) dinfo.instruction1 = command_out.read();
    if (bitmap & DINFO_BIT_INST2) dinfo.instruction2 = command_out.read();
    if (bitmap & DINFO_BIT_SP) dinfo.sp = command_out.read();
}

void show_debug_info(word_t bitmap, debug_s& dinfo, bool header) {
    if (header) {
        if (bitmap) std::cout << "|";
        if (bitmap & DINFO_BIT_CYCLE) std::cout << " cycle |";
        if (bitmap & DINFO_BIT_PC)  std::cout << "  PC   |";
        if (bitmap & DINFO_BIT_REGA)  std::cout << "   A   |";
        if (bitmap & DINFO_BIT_REGD)  std::cout << "   D   |";
        if (bitmap & DINFO_BIT_ALUO)  std::cout << "  ALU  |";
        if (bitmap & DINFO_BIT_INST1) std::cout << " inst1 |";
        if (bitmap & DINFO_BIT_INST2)  std::cout << " inst2 |";
        if (bitmap & DINFO_BIT_WOUT) std::cout << " wout |";
        if (bitmap & DINFO_BIT_OUTM) std::cout << " outM  |";
        if (bitmap & DINFO_BIT_ADDRM)  std::cout << " addrM |";
        if (bitmap & DINFO_BIT_SP)  std::cout << "   SP  |";
        if (bitmap) std::cout << std::endl;
    }
    char buf[256];
    static bool onlyStackShow = false;
    static int pcBreak = 25788;
    static int cycleBreak = 5000000;

    if (onlyStackShow && ((dinfo.write_out == 0) || (dinfo.addressM != 0))) return; // only stack
    //if ((pcBreak == -1) || (pcBreak == dinfo.pc)) { pcBreak=-1;return;}
    if ((cycleBreak == -1) || (cycleBreak == dinfo.cycle)) { cycleBreak=-1;return;}

    if (bitmap) std::cout << "|";
    if (bitmap & DINFO_BIT_CYCLE) {sprintf(buf, "%6d |", dinfo.cycle); std::cout << buf; }
    //if (bitmap & DINFO_BIT_PC) {sprintf(buf, "0x%04x |", dinfo.pc); std::cout << buf; }
    if (bitmap & DINFO_BIT_PC) {sprintf(buf, "%6d |", dinfo.pc); std::cout << buf; }
    //if (bitmap & DINFO_BIT_REGA) {sprintf(buf, "0x%04x |", dinfo.regA); std::cout << buf; }
    if (bitmap & DINFO_BIT_REGA) {sprintf(buf, "%6d |", dinfo.regA); std::cout << buf; }
    //if (bitmap & DINFO_BIT_REGD) {sprintf(buf, "0x%04x |", dinfo.regD); std::cout << buf; }
    if (bitmap & DINFO_BIT_REGD) {sprintf(buf, "%6d |", dinfo.regD); std::cout << buf; }
    if (bitmap & DINFO_BIT_ALUO) {sprintf(buf, "0x%04x |", dinfo.alu_out); std::cout << buf; }
    if (bitmap & DINFO_BIT_INST1) {sprintf(buf, "0x%04x |", dinfo.instruction1); std::cout << buf; }
    if (bitmap & DINFO_BIT_INST2) {sprintf(buf, "0x%04x |", dinfo.instruction2); std::cout << buf; }
    if (bitmap & DINFO_BIT_WOUT) {sprintf(buf, "%4d  |", dinfo.write_out.to_int()); std::cout << buf; }
    if (bitmap & DINFO_BIT_OUTM) {sprintf(buf, "0x%04x |", dinfo.outM); std::cout << buf; }
    if (bitmap & DINFO_BIT_ADDRM) {sprintf(buf, "0x%04x |", dinfo.addressM); std::cout << buf; }
    //if (bitmap & DINFO_BIT_SP) {sprintf(buf, "0x%04x |", dinfo.sp); std::cout << buf; }
    if (bitmap & DINFO_BIT_SP) {sprintf(buf, "%6d |", dinfo.sp); std::cout << buf; }
    
    if (bitmap & (DINFO_BIT_INST1 | DINFO_BIT_INST2)) {std::cout << " " << disassemble(dinfo.instruction1) /*<< " " << disassemble(dinfo.instruction2)*/;}
    if (bitmap) std::cout << std::flush;
    if (bitmap) std::cout << std::endl;
}

static void disp_out_via_uart(uart_comm& uart_comm, word_t addrM, word_t dataM) {
	if (addrM >= 0x4000) {
		char send_chars[10];
		unsigned short addr = addrM - 0x4000;
		send_chars[0] = '!';
		for (int i = 1; i <= 4; i++) {
			char temp = ((addr >> (4 - i) * 4) & 0xF);
			send_chars[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
		}
		for (int i = 5; i <= 8; i++) {
			char temp = ((dataM >> (8 - i) * 4) & 0xF);
			send_chars[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
		}
		send_chars[9] = '\n';
		send_chars[10] = '\0';
        uart_comm.write_data(send_chars, strlen(send_chars));
	}
}

int compare(hls::stream<word_t>& command_in, hls::stream<word_t>& command_out, bool header, uart_comm& uart_comm) {

    word_t bitmap = DINFO_BIT_CYCLE | DINFO_BIT_PC | DINFO_BIT_REGA | DINFO_BIT_REGD |
    		        DINFO_BIT_INST1 | DINFO_BIT_INST2 | DINFO_BIT_WOUT |
    		        DINFO_BIT_ADDRM | DINFO_BIT_OUTM | DINFO_BIT_SP;
    debug_s dinfo;
    get_debug_info(command_in, command_out, bitmap, dinfo);
    show_debug_info(bitmap, dinfo, header);
    if (dinfo.write_out) {
    	disp_out_via_uart(uart_comm, dinfo.addressM, dinfo.outM);
    }
    return 0;
}

static word_t make_hex_bin(const char* hex_chars) {
	word_t hex_data = 0;
	for (int i = 0; i < 4; i++) {
		if ((hex_chars[i] >= '0') && (hex_chars[i] <= '9')) hex_data += ((hex_chars[i]-'0') << (3-i)*4);
		else if ((hex_chars[i] >= 'a') && (hex_chars[i] <= 'f')) hex_data += ((hex_chars[i]-'a'+10) << (3-i)*4);
		else if ((hex_chars[i] >= 'A') && (hex_chars[i] <= 'F')) hex_data += ((hex_chars[i]-'A'+10) << (3-i)*4);
	}
	return hex_data;
}

static void make_hex_chars(word_t hex_data, char hex_chars[6]) {
	for (int i = 0; i < 4; i++) {
		char temp = ((hex_data >> (3 - i) * 4) & 0xF);
		hex_chars[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'a';
	}
	hex_chars[4] = '\n';
	hex_chars[5] = '\0';
}

static void uart_bridge(uart_comm& uart_comm, hls::stream<word_t>& command_in, hls::stream<word_t>& command_out) {
	char read_buf[4] = {0};
	DWORD bytes_read;
	while (1) {
		if (uart_comm.read_data(read_buf, sizeof(read_buf), bytes_read)) {
			if (bytes_read == 4) {
				if (read_buf[0] == 'Z') {
					// 実行
					cpu_wrapper(command_in, command_out);
					while (command_out.empty()) {}
					while (!command_out.empty()) {
						word_t num_retvals = command_out.read();
						char buf[16];
						sprintf(buf, "nrv: %04x", num_retvals);
						std::cout << buf << std::endl;
						char send_chars[6];
						make_hex_chars(num_retvals, send_chars);
						uart_comm.write_data(send_chars, strlen(send_chars));

						for (int i = 0; i < num_retvals; i++) {
							word_t read_data = command_out.read();
							int data = read_data.to_uint();
							sprintf(buf, "rcv: %04x", data);
							std::cout << buf << std::endl;
							make_hex_chars(data, send_chars);
							uart_comm.write_data(send_chars, strlen(send_chars));
						}
						word_t reason = command_out.read();
						sprintf(buf, "rsn: %04x", reason);
						std::cout << buf << std::endl;
						make_hex_chars(reason, send_chars);
				        uart_comm.write_data(send_chars, strlen(send_chars));

				        if (reason == BREAK_REASON_DISP) {
				        	word_t addr = command_out.read();
				        	word_t data = command_out.read();
				        	disp_out_via_uart(uart_comm, addr, data);
				        }
					}
				} else {
					word_t send_data = make_hex_bin(read_buf);
					char buf[16];
					sprintf(buf, "snd: %04x", send_data);
					std::cout << buf << std::endl;
					command_in.write(send_data);
				}
			}
		}
	}
}

int main() {

	uart_comm uart_comm("\\\\.\\"USE_COM);  // COM2ポートを開く

    // CPU interface signals
    hls::stream<word_t> command_in;
    hls::stream<word_t> command_out;

 #if 1
    // Reset CPU
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_RESET | RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();

    // Write to ROM
    // Read ROM content from file
    read_rom_file("rom_string_test.bin", command_in, command_out);

    // For rect example
    command_in.write(WRITE_TO_DRAM);
    command_in.write(0x0000);
    command_in.write(0x0014);

    command_in.write(SET_BREAK_CONDITION);
    command_in.write(BREAK_CONDITION_BIT_DISPOUT);

    command_in.write(STEP_EXECUTION);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
#endif

#if 1
    uart_bridge(uart_comm, command_in, command_out);

#else

    // Run CPU cycle
    #if 0
    // Run through
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
    #elif 0
    // Step debugging
    for (int i = 0; i < 5000000; i++) {
        command_in.write(STEP_EXECUTION);
        cpu_wrapper(command_in, command_out);
        DUMMY_READ();
        compare(command_in, command_out, (i == 0), uart_comm);
   }
    #else
    // Run until mem out
    command_in.write(SET_BREAK_CONDITION);
    command_in.write(BREAK_CONDITION_BIT_DISPOUT);
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    for (int i = 0; i < 2000; i++) {
        command_in.write(NORMAL_OPERATION);
        cpu_wrapper(command_in, command_out);
        DUMMY_READ();
        compare(command_in, command_out, (i == 0), uart_comm);
     }
    #endif

    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(READ_FROM_DRAM);
    command_in.write(0x0007);
    cpu_wrapper(command_in, command_out);
    DUMMY_READ();
#endif
    return 0;
}
