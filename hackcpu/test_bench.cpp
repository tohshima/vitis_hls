#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_task.h>
#include <hls_stream.h>
#include "revasm.hpp"
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h
#ifndef SIM_CPU_WRAPPER
#ifdef USE_HACKCPU_UART
#include "hackcpu_uart.hpp"
#else
#include "uart_if.hpp"
#include "uart_in_task.hpp"
#include "comp_task.hpp"
#include "peripheral_task.hpp"
#include "uart_out_task.hpp"
#endif
#endif

#ifndef SIM_CPU_WRAPPER
// debug using external controller via uart
int main() {
    #ifdef USE_HACKCPU_UART

    unsigned int uart_reg[4] = {0};
    return hackcpu_uart(uart_reg);

    #else
	bool start = true;
	//ap_uint<1> interrupt = 0;
    volatile unsigned int uart_reg[4] = {0};
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
    return 0;
    #endif
}

#else // #ifndef SIM_CPU_WRAPPER
// only debug cpu_wrapper
void read_rom_file(
    const std::string& filename,
    hls::stream<word_t>& command_in,
    hls::stream<word_t>& command_out,
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out
) {
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
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();
}

void get_debug_info(
    hls::stream<word_t>& command_in, 
    hls::stream<word_t>& command_out, 
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
    word_t bitmap, 
    debug_s& dinfo
) {
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(GET_DEBUG_INFO);
    command_in.write(bitmap);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
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

int compare(
    hls::stream<word_t>& command_in, 
    hls::stream<word_t>& command_out, 
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out,
    bool header
) {

    word_t bitmap = DINFO_BIT_CYCLE | DINFO_BIT_PC | DINFO_BIT_REGA | DINFO_BIT_REGD |
    		        DINFO_BIT_INST1 | DINFO_BIT_INST2 | DINFO_BIT_WOUT |
    		        DINFO_BIT_ADDRM | DINFO_BIT_OUTM | DINFO_BIT_SP;
    debug_s dinfo;
    get_debug_info(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, bitmap, dinfo);
    show_debug_info(bitmap, dinfo, header);
    return 0;
}

int main() {

    // CPU interface signals
    hls::stream<word_t> command_in;
    hls::stream<word_t> command_out;
    hls::stream<word_t> interrupt_in;
    hls::stream<addr_t> peripheral_raddr_out;
    hls::stream<word_t> peripheral_rdata_in;
    hls::stream<addr_t> peripheral_waddr_out;
    hls::stream<word_t> peripheral_wdata_out;

    // Reset CPU
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_RESET | RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();
    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();

    // Write to ROM
    // Read ROM content from file
    //read_rom_file("rom_string_test.bin", command_in, command_out);
    read_rom_file("rom_pong.bin", command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);

    // For rect example
    command_in.write(WRITE_TO_DRAM);
    command_in.write(0x0000);
    command_in.write(0x0014);

    //command_in.write(SET_BREAK_CONDITION); // obsolete
    //command_in.write(BREAK_CONDITION_BIT_DISPOUT | BREAK_CONDITION_BIT_KEYIN);
    //command_in.write(0x8000);

//    command_in.write(STEP_EXECUTION);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();

    // Run CPU cycle
    #if 1
    // Run through
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();
    #elif 0
    // Step debugging
    for (int i = 0; i < 5000000; i++) {
        command_in.write(STEP_EXECUTION);
        cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
        DUMMY_READ();
        compare(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, (i == 0));
    }
    #else
    // Run until mem out
    command_in.write(SET_BREAK_CONDITION);
    command_in.write(BREAK_CONDITION_BIT_DISPOUT);
    command_in.write(SET_RESET_CONFIG);
    command_in.write(0);
    for (int i = 0; i < 2000; i++) {
        command_in.write(NORMAL_OPERATION);
        cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
        DUMMY_READ();
        compare(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out, (i == 0));
    }
    #endif

    command_in.write(SET_RESET_CONFIG);
    command_in.write(RESET_BIT_HALT);
    command_in.write(READ_FROM_DRAM);
    command_in.write(0x0007);
    cpu_wrapper(command_in, command_out, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
    DUMMY_READ();
    return 0;
}
#endif //#ifndef SIM_CPU_WRAPPER
