#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_stream.h>
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h

void read_rom_file(const std::string& filename,
             hls::stream<word_t>& command_in,
             hls::stream<word_t>& command_out) {
    const int rom_size = 1 << ADDR_WIDTH;
    //for (int i = 0; i < rom_size; i++) ROM[i] = INST_FETCH_STOP;

    std::ifstream file(filename);
    std::string line;
    
    int addr = 0;
    while (std::getline(file, line)) {
        word_t instruction = 0;
        for (char c : line) {
            instruction = (instruction << 1) | (c - '0');
        }
        command_in.write(WRITE_TO_IRAM);
        command_in.write(addr++);
        command_in.write(instruction);
        cpu_wrapper(command_in, command_out);
        //ROM[addr++] = instruction;
        printf("%s %08x\n", line.c_str(), instruction.to_ushort());
    }
}

int compare() {

}

int main() {

    // CPU interface signals
    hls::stream<word_t> command_in;
    hls::stream<word_t> command_out;

    // Reset CPU
    command_in.write(ASSERT_RESET);
    cpu_wrapper(command_in, command_out);
    command_in.write(DEASSERT_RESET);
    cpu_wrapper(command_in, command_out);

    // Write to ROM
    // Read ROM content from file
    read_rom_file("rom.bin", command_in, command_out);

    // Run CPU cycle
    #if 0
    // Run through
    command_in.write(DEASSERT_HALT);
    cpu_wrapper(command_in, command_out);
    #else
    // Step debugging
    for (int i = 0; i < 20; i++) {
        command_in.write(STEP_EXECUTION);
        cpu_wrapper(command_in, command_out);
        command_in.write(ASSERT_HALT);
    }
    #endif

    command_in.write(ASSERT_HALT);
    command_in.write(READ_FROM_DRAM);
    command_in.write(0x0007);
    cpu_wrapper(command_in, command_out);
    word_t val = command_out.read();
    std::cout << "Read val: " << val << std::endl;
    if (val != 0x0008) return 1;
    return 0;
}