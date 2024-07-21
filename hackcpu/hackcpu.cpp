#include "hackcpu.hpp"

// Predefined ROM
static word_t rom_content[] = {
    0b0000000000000010,
    0b1110110000010000,
    0b0000000000000011,
    0b1110000010010000,
    0b0000000000000000,
    0b1110001100001000,
};

#define ROM_SIZE (sizeof(rom_content)/sizeof(rom_content[0]))

// CPU function
static void cpu(word_t rom[ROM_SIZE],
         ap_uint<1>& reset, ap_uint<1>& write_out, word_t& outM, addr_t& addressM, word_t& pc,
         word_t& debug_A, word_t& debug_D, word_t& debug_instruction) {
#pragma HLS INTERFACE ap_memory port=rom
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
#pragma HLS BIND_STORAGE variable=ram type=RAM_2P impl=BRAM

    // CPU registers
    static word_t A = 0, D = 0;
    static addr_t PC = 0;

    if (reset) {
        A = 0;
        D = 0;
        PC = 0;
        // Update debug information
        debug_A = A;
        debug_D = D;
        debug_instruction = 0; // No instruction on reset
        pc = PC;
        return;
    }

    // Fetch instruction from ROM
    word_t instruction = rom[PC];

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

    // Update debug information
    debug_A = A;
    debug_D = D;
    debug_instruction = instruction;
    pc = PC;
}

void cpu_wrapper(ap_uint<1>& write_out, word_t& outM, addr_t& addressM, word_t& pc,
                 word_t& debug_A, word_t& debug_D, word_t& debug_instruction) {
#pragma HLS INTERFACE ap_none port=write_out
#pragma HLS INTERFACE ap_none port=outM
#pragma HLS INTERFACE ap_none port=addressM
#pragma HLS INTERFACE ap_none port=pc
#pragma HLS INTERFACE ap_none port=debug_A
#pragma HLS INTERFACE ap_none port=debug_D
#pragma HLS INTERFACE ap_none port=debug_instruction
#pragma HLS BIND_STORAGE variable=rom_content type=ROM_2P

    // CPU interface signals
    static ap_uint<1> reset = 1;

    // Reset CPU
    cpu(rom_content, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_instruction);

    // Run simulation
    reset = 0;
    for (int cycle = 0; cycle < ROM_SIZE; ++cycle) {
        // Run CPU cycle
        cpu(rom_content, reset, write_out, outM, addressM, pc, debug_A, debug_D, debug_instruction);
    }
}