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

void read_rom_file(const std::string& filename,
             hls::stream<word_t>& command_in,
             hls::stream<word_t>& command_out,
             hls::stream<unsigned int> uart_regs[4]) {
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
    cpu_wrapper(command_in, command_out, uart_regs);
}

void get_debug_info(hls::stream<word_t>& command_in, hls::stream<word_t>& command_out, word_t bitmap, debug_s& dinfo, hls::stream<unsigned int> uart_regs[4]) {
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(GET_DEBUG_INFO);
    command_in.write(bitmap);
    cpu_wrapper(command_in, command_out, uart_regs);
    
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

int compare(hls::stream<word_t>& command_in, hls::stream<word_t>& command_out, bool header, hls::stream<unsigned int> uart_regs[4]) {

    word_t bitmap = DINFO_BIT_CYCLE | DINFO_BIT_PC | DINFO_BIT_REGA | DINFO_BIT_REGD |
    		        DINFO_BIT_INST1 | DINFO_BIT_INST2 | DINFO_BIT_WOUT |
    		        DINFO_BIT_ADDRM | DINFO_BIT_OUTM | DINFO_BIT_SP;
    debug_s dinfo;
    get_debug_info(command_in, command_out, bitmap, dinfo, uart_regs);
    show_debug_info(bitmap, dinfo, header);
    return 0;
}

int main() {

	uart_comm uart_comm("\\\\.\\"USE_COM);  // COM2ポートを開く

    // CPU interface signals
    hls::stream<word_t> command_in;
    hls::stream<word_t> command_out;
    hls::stream<unsigned int> uart_regs[4];

    // Reset CPU
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_RESET | RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out, uart_regs);
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out, uart_regs);

    // Write to ROM
    // Read ROM content from file
    //read_rom_file("rom.bin", command_in, command_out, uart_regs);
    read_rom_file("rom_string_test.bin", command_in, command_out, uart_regs);
    command_in.write(WRITE_TO_DRAM);

    // For rect example
    command_in.write(0x0000);
    command_in.write(0x0014);

    // Run CPU cycle
    #if 0
    // Run through
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    cpu_wrapper(command_in, command_out, uart_regs);
    #elif 1
    // Step debugging
    for (int i = 0; i < 5000000; i++) {
        command_in.write(STEP_EXECUTION);
        cpu_wrapper(command_in, command_out, uart_regs);
        compare(command_in, command_out, (i == 0), uart_regs);
        if (!uart_regs[1].empty()) {
        	char str[32] = {0};
        	int s = 0;
            while (!uart_regs[1].empty()) {
            	char c = uart_regs[1].read();
            	str[s++] = c;
            	std::cout << c;
            }
            std::cout << std::endl;
            uart_comm.write_data(str, strlen(str));
        }
   }
    #else
    // Run until mem out
    command_in.write(SET_BREAK_CONDITION);
    command_in.write(BREAK_CONDITION_BIT_DISPOUT);
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    for (int i = 0; i < 200; i++) {
        cpu_wrapper(command_in, command_out, uart_regs);
        compare(command_in, command_out, (i == 0), uart_regs);
     }
    #endif

    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(READ_FROM_DRAM);
    command_in.write(0x0007);
    cpu_wrapper(command_in, command_out, uart_regs);
    word_t val = command_out.read();
    std::cout << "Read val: " << val << std::endl;
    //if (val != 0x0008) return 1;
    return 0;
}
