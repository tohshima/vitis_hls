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
        printf("%s %08x\n", line.c_str(), instruction);
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
    word_t debug_ALU;
    word_t debug_instruction;

    // Reset CPU
    cpu(rom_in, rom_addr, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_ALU, debug_instruction);

    // Run simulation
    reset = 0;
    char buf[512];
    for (int cycle = 0; cycle < 100; ++cycle) {
        // Handle ROM read request
        if (!rom_addr.empty()) {
            axi_word_t addr_req = rom_addr.read();
            addr_t rom_addr_val = addr_req.data;
            if (rom_addr_val < rom_content.size()) {
                axi_word_t instruction;
                instruction.data = rom_content[rom_addr_val];
                rom_in.write(instruction);
                sprintf(buf, "Cycle %6d: RA=%04x, RD=%04x",
                    cycle, rom_addr_val.data, instruction.to_ushort());
                std::cout << buf;
            } else {
                std::cout << "Error: ROM address out of bounds" << std::endl;
                break;
            }
        }

        // Run CPU cycle
        cpu(rom_in, rom_addr, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_ALU, debug_instruction);

        // Print CPU state
        sprintf(buf, ", PC=%04x, A=%04x, D=%04x, ALU=%04x, Inst=%04x",
            pc.to_ushort(),debug_A.to_ushort(),debug_D.to_ushort(), 
            debug_ALU.to_ushort(), debug_instruction.to_ushort());
        std::cout << buf << std::endl;
        if (write_out) {
            std::cout << ", Writing " << outM << " to RAM[" << addressM << "]";
        }
        std::cout << std::endl;
    }

    return 0;
}