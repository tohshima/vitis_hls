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
    hls::stream<debug_t> debug;

    // Reset CPU
    cpu(ROM, reset, debug);

    // Run simulation
    reset = 0;
    for (int cycle = 0; cycle < 20; ++cycle) {

        // Run CPU cycle
        cpu(ROM, reset, debug);

        // Print CPU state
        while (!debug.empty()) {
            char buf[512];
            debug_t dinfo = debug.read();
            sprintf(buf, "Cycle %6d: PC=%04x, A=%04x, D=%04x, ALU=%04x, Inst=%04x",
                dinfo.data.cycle, dinfo.data.pc.to_ushort(),dinfo.data.regA.to_ushort(),dinfo.data.regD.to_ushort(), 
                dinfo.data.alu_out.to_ushort(), dinfo.data.instruction.to_ushort());
            std::cout << buf;
            if (dinfo.data.write_out) {
                sprintf(buf, " Write %04x to RAM[%04x]",dinfo.data.outM.to_ushort(), dinfo.data.addressM.to_ushort());
                std::cout << buf;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}