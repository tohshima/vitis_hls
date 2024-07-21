#include "hackcpu.hpp"

// CPU function
void cpu(hls::stream<axi_word_t>& rom_in, hls::stream<axi_word_t>& rom_addr,
         ap_uint<1>& reset, ap_uint<1>& write_out, word_t& outM, addr_t& addressM, word_t& pc,
         word_t& debug_A, word_t& debug_D, word_t& debug_instruction) {
#pragma HLS INTERFACE axis port=rom_in
#pragma HLS INTERFACE axis port=rom_addr
#pragma HLS INTERFACE ap_none port=reset
#pragma HLS INTERFACE ap_none port=write_out
#pragma HLS INTERFACE ap_none port=outM
#pragma HLS INTERFACE ap_none port=addressM
#pragma HLS INTERFACE ap_none port=pc
#pragma HLS INTERFACE ap_none port=debug_A
#pragma HLS INTERFACE ap_none port=debug_D
#pragma HLS INTERFACE ap_none port=debug_instruction

    // Internal RAM
    static word_t ram[1 << ADDR_WIDTH];
#pragma HLS RESOURCE variable=ram core=RAM_2P_BRAM

    // CPU registers
    static word_t A = 0, D = 0;
    static addr_t PC = 0;

    if (reset) {
        A = 0;
        D = 0;
        PC = 0;
        // Request first instruction
        axi_word_t rom_addr_out;
        rom_addr_out.data = PC;
        rom_addr.write(rom_addr_out);
        return;
    }

    // Fetch instruction from ROM
    axi_word_t rom_addr_out;
    rom_addr_out.data = PC;
    rom_addr.write(rom_addr_out);
    
    axi_word_t instruction_in = rom_in.read();
    word_t instruction = instruction_in.data;

    // Decode and execute instruction
    bool is_a_instruction = (instruction[15] == 0);
    
    if (is_a_instruction) {
        A = instruction;
    } else {
        // C-instruction
        word_t alu_out;
        bool jump = false;

        // ALU computation
        word_t x = D;
        word_t y = instruction[12] ? ram[A] : A;  // A/M bit

        switch (instruction(11, 6)) {
            case 0b101010: alu_out = 0; break;                    // 0
            case 0b111111: alu_out = 1; break;                    // 1
            case 0b111010: alu_out = -1; break;                   // -1
            case 0b001100: alu_out = x; break;                    // D
            case 0b110000: alu_out = y; break;                    // A/M
            case 0b001101: alu_out = ~x; break;                   // !D
            case 0b110001: alu_out = ~y; break;                   // !A/M
            case 0b001111: alu_out = -x; break;                   // -D
            case 0b110011: alu_out = -y; break;                   // -A/M
            case 0b011111: alu_out = x + 1; break;                // D+1
            case 0b110111: alu_out = y + 1; break;                // A/M+1
            case 0b001110: alu_out = x - 1; break;                // D-1
            case 0b110010: alu_out = y - 1; break;                // A/M-1
            case 0b000010: alu_out = x + y; break;                // D+A/M
            case 0b010011: alu_out = x - y; break;                // D-A/M
            case 0b000111: alu_out = y - x; break;                // A/M-D
            case 0b000000: alu_out = x & y; break;                // D&A/M
            case 0b010101: alu_out = x | y; break;                // D|A/M
            default: alu_out = 0; // Undefined behavior
        }

        // Destination
        if (instruction[5]) A = alu_out;
        if (instruction[4]) D = alu_out;
        if (instruction[3]) {
            ram[A] = alu_out;
            write_out = 1;
            outM = alu_out;
            addressM = A;
        } else {
            write_out = 0;
        }

        // Jump condition
        bool pos = (alu_out > 0);
        bool zero = (alu_out == 0);
        bool neg = (alu_out < 0);

        if ((instruction[2] && neg) || (instruction[1] && zero) || (instruction[0] && pos)) {
            jump = true;
        }

        // Update PC
        if (jump) {
            PC = A;
        } else {
            PC++;
        }
    }

    if (!is_a_instruction) {
        PC++;
    }
    
    // Update debug information
    pc = PC;
    debug_A = A;
    debug_D = D;
    debug_instruction = instruction;
}
