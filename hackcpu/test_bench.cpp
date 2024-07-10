#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_stream.h>
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h

void read_rom_file(const std::string& filename, word_t ROM[]) {
    const int rom_size = 1 << ADDR_WIDTH;
    for (int i = 0; i < rom_size; i++) ROM[i] = INST_FETCH_STOP;

    std::ifstream file(filename);
    std::string line;
    
    int addr = 0;
    while (std::getline(file, line)) {
        word_t instruction = 0;
        for (char c : line) {
            instruction = (instruction << 1) | (c - '0');
        }
        ROM[addr++] = instruction;
        printf("%s %08x\n", line.c_str(), instruction);
    }
}

int main() {
    // Read ROM content from file
    word_t ROM[1 << ADDR_WIDTH];
    read_rom_file("rom.bin", ROM);

    // CPU interface signals
    ap_uint<1> reset = 1;
    ap_uint<1> write_out;
    word_t outM;
    addr_t addressM;
    word_t pc;
    word_t debug_A;
    word_t debug_D;
    word_t debug_ALU;
    word_t debug_instruction;

    // Reset CPU
    cpu(ROM, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_ALU, debug_instruction);

    // Run simulation
    reset = 0;
    for (int cycle = 0; cycle < 100; ++cycle) {

        // Run CPU cycle
        cpu(ROM, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_ALU, debug_instruction);

        // Print CPU state
        char buf[512];
        sprintf(buf, "Cycle %6d: PC=%04x, A=%04x, D=%04x, ALU=%04x, Inst=%04x",
            cycle, pc.to_ushort(),debug_A.to_ushort(),debug_D.to_ushort(), 
            debug_ALU.to_ushort(), debug_instruction.to_ushort());
        std::cout << buf;
        if (write_out) {
            sprintf(buf, "Write %04x to RAM[%04x]",outM.to_ushort(), addressM.to_ushort());
            std::cout << buf;
        }
        std::cout << std::endl;
    }

    return 0;
}