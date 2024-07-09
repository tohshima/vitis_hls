#include "hackcpu.hpp"

// CPU function
void cpu(hls::stream<axi_word_t>& rom_in, hls::stream<axi_word_t>& rom_addr,
         ap_uint<1>& reset, ap_uint<1>& write_out, word_t& outM, addr_t& addressM, word_t& pc,
         word_t& debug_A, word_t& debug_D, word_t& debug_ALU, word_t& debug_instruction) {
#pragma HLS INTERFACE axis port=rom_in
#pragma HLS INTERFACE axis port=rom_addr
#pragma HLS INTERFACE ap_none port=reset
#pragma HLS INTERFACE ap_none port=write_out
#pragma HLS INTERFACE ap_none port=outM
#pragma HLS INTERFACE ap_none port=addressM
#pragma HLS INTERFACE ap_none port=pc
#pragma HLS INTERFACE ap_none port=debug_A
#pragma HLS INTERFACE ap_none port=debug_D
#pragma HLS INTERFACE ap_none port=debug_ALU
#pragma HLS INTERFACE ap_none port=debug_instruction

    // Internal RAM
    static word_t ram[1 << ADDR_WIDTH];
#pragma HLS BIND_STORAGE variable=ram type=RAM_2P impl=BRAM

    // CPU registers
    static word_t A = 0, D = 0;
    static addr_t PC = 0;

    static axi_word_t rom_addr_out;

    if (reset) {
        A = 0;
        D = 0;
        PC = 0;
        rom_addr_out.data = PC;
        rom_addr.write(rom_addr_out);
        return;
    }

    axi_word_t instruction_in = rom_in.read();
    word_t instruction = instruction_in.data;

    // Decode and execute instruction
    bool is_a_instruction = (instruction[15] == 0);
    
    if (is_a_instruction) {
        A = instruction;
        PC++;
    } else {
        // C-instruction
        word_t alu_out;
        bool jump = false;

        // ALU computation
        switch (instruction(12, 10)) {
            case 0b000: alu_out = D & A; break;
            case 0b001: alu_out = D | A; break;
            case 0b010: alu_out = D + A; break;
            case 0b011: alu_out = D - A; break;
            case 0b100: alu_out = D & ram[A]; break;
            case 0b101: alu_out = D | ram[A]; break;
            case 0b110: alu_out = D + ram[A]; break;
            case 0b111: alu_out = D - ram[A]; break;
            default: alu_out = 0;
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
    
        debug_ALU = alu_out;
    }
    // Fetch instruction from ROM for next cycle
    rom_addr_out.data = PC;
    rom_addr.write(rom_addr_out);
        
    // Update debug information
    pc = PC;
    debug_A = A;
    debug_D = D;
    debug_instruction = instruction;
}
