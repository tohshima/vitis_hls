#include "hackcpu.hpp"

static void comp(word_t instruction, word_t A, word_t D, word_t ram[1 << ADDR_WIDTH], word_t& alu_out) {
#pragma HLS inline
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
}

static void get_destination(word_t instruction, word_t alu_out, 
    word_t& A, word_t& D, word_t ram[1 << ADDR_WIDTH], ap_uint<1>& write_out, word_t& outM, addr_t& addressM) {
#pragma HLS inline

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
}

static bool get_jump_condition(word_t alu_out, word_t instruction) {
#pragma HLS inline
    bool pos = (alu_out > 0);
    bool zero = (alu_out == 0);
    bool neg = (alu_out < 0);

    if ((instruction[2] && neg) || (instruction[1] && zero) || (instruction[0] && pos)) {
        return true;
    }
    return false;
}

static void updatePC(bool jump, addr_t& PC, word_t& A, word_t instruction) {
#pragma HLS inline
    if (jump) {
        PC = A;
    } else {
        if (instruction != INST_FETCH_STOP) {
            PC++;
        }
    }
}

// CPU function
// ToDo: rom double word, ram_read axilite_s?), debug->stream
void cpu(word_t rom[1 << ADDR_WIDTH],
         ap_uint<1>& reset, hls::stream<debug_t>& debug) {
#pragma HLS INTERFACE ap_memory port=rom
#pragma HLS INTERFACE ap_none port=reset
#pragma HLS INTERFACE axis port=debug

// CPU registers
typedef struct {
    word_t A;
    word_t D;
    addr_t PC;
} regs_s;
static regs_s Regs;

// Internal RAM
static word_t ram[1 << ADDR_WIDTH];
#pragma HLS BIND_STORAGE variable=ram type=RAM_2P impl=BRAM

    if (reset) {
        Regs.A = 0;
        Regs.D = 0;
        Regs.PC = 0;
        return;
    } else {
        for (int cycle = 0; ; cycle++) {
            // C-instruction
            word_t alu_out = 0;
            ap_uint<1> write_out = 0;
            word_t outM = 0;
            addr_t addressM = 0;

            word_t instruction = rom[Regs.PC];

            // Decode and execute instruction
            bool is_a_instruction = (instruction[15] == 0);
            
            if (is_a_instruction) {
                Regs.A = instruction;
                Regs.PC++;
            } else {
                bool jump = false;

                // ALU computation
                comp(instruction, Regs.A, Regs.D, ram, alu_out);
                
                // Destination
                get_destination(instruction, alu_out,
                    Regs.A, Regs.D, ram, write_out, outM, addressM);

                // Jump condition
                jump = get_jump_condition(alu_out, instruction);

                // Update PC
                updatePC(jump, Regs.PC, Regs.A, instruction);                
            }

            // Update debug information
            debug_t dinfo;
            dinfo.data.cycle = cycle;
            dinfo.data.write_out = write_out;
            dinfo.data.outM = outM;
            dinfo.data.addressM = addressM;
            dinfo.data.pc = Regs.PC;
            dinfo.data.pc = Regs.PC;
            dinfo.data.regA = Regs.A;
            dinfo.data.regD = Regs.D;
            dinfo.data.instruction = instruction;
            debug.write(dinfo);

            if (instruction == INST_FETCH_STOP) break;
        }
    }
}
