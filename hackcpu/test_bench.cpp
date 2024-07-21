#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_stream.h>
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h

void read_rom_file(const std::string& filename, std::vector<word_t>& rom_content) {
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        word_t instruction = 0;
        for (char c : line) {
            instruction = (instruction << 1) | (c - '0');
        }
        rom_content.push_back(instruction);
    }
}

int main() {
    // Read ROM content from file
    std::vector<word_t> rom_content;
    read_rom_file("rom.bin", rom_content);

    // Create AXI streams for ROM interface
    hls::stream<axi_word_t> rom_in;
    hls::stream<axi_word_t> rom_addr;

    // CPU interface signals
    ap_uint<1> reset = 1;
    ap_uint<1> write_out;
    word_t outM;
    addr_t addressM;
    word_t pc;
    word_t debug_A;
    word_t debug_D;
    word_t debug_instruction;

    // Reset CPU
    cpu(rom_in, rom_addr, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_instruction);

    // Run simulation
    reset = 0;
    for (int cycle = 0; cycle < 100; ++cycle) {
        // Handle ROM read request
        if (!rom_addr.empty()) {
            axi_word_t addr_req = rom_addr.read();
            addr_t rom_addr_val = addr_req.data;
            
            if (rom_addr_val < rom_content.size()) {
                axi_word_t instruction;
                instruction.data = rom_content[rom_addr_val];
                rom_in.write(instruction);
            } else {
                std::cout << "Error: ROM address out of bounds" << std::endl;
                break;
            }
        }

        // Run CPU cycle
        cpu(rom_in, rom_addr, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_instruction);

        // Print CPU state
        std::cout << "Cycle " << std::setw(3) << cycle 
                  << ": PC = " << std::setw(5) << pc
                  << ", A = " << std::setw(5) << debug_A
                  << ", D = " << std::setw(5) << debug_D
                  << ", Instruction = 0x" << std::hex << std::setw(4) << std::setfill('0') << debug_instruction << std::dec << std::setfill(' ');
        if (write_out) {
            std::cout << ", Writing " << outM << " to RAM[" << addressM << "]";
        }
        std::cout << std::endl;
    }

    return 0;
}