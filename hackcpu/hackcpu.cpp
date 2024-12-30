#include "hackcpu.hpp"

static void comp_core(word_t instruction, word_t x, word_t y, word_t &alu_out) {
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

static void comp(word_t instruction, 
                 word_t A, word_t D,
                 word_t& alu_out) {
#pragma HLS inline
    word_t x = D;
    word_t y = A;
    comp_core(instruction, x, y, alu_out);
}

static void compM(word_t instruction, word_t A, word_t D, word_t ram[IRAM_SIZE],
                  word_t &alu_out) {
#pragma HLS inline
    word_t x = D;
    word_t y = ram[A];
    comp_core(instruction, x, y, alu_out);
}

static void get_destination(word_t instruction, word_t alu_out,
    word_t& A, word_t& D,
    word_t ram[DRAM_SIZE], 
    ap_uint<1>& write_out, word_t& outM, addr_t addressM) {
#pragma HLS inline
    if (instruction[3]) {
        ram[addressM] = alu_out;
        write_out = 1;
        outM = alu_out;
    } else {
        write_out = 0;
    }
    if (instruction[4]) D = alu_out;
    if (instruction[5]) A = alu_out;
}

static bool get_jump_condition(word_t alu_out, word_t instruction) {
#pragma HLS inline
    bool pos = (alu_out[alu_out.width-1] == 0) && (alu_out != 0);
    bool zero = (alu_out == 0);
    bool neg = (alu_out[alu_out.width-1] == 1);

    if ((instruction[2] && neg) || (instruction[1] && zero) || (instruction[0] && pos)) {
        return true;
    }
    return false;
}

static void updatePC(bool jump, addr_t& PC, word_t& A,
                     word_t instruction) {
#pragma HLS inline
    if (jump) {
        PC = A;
    } else {
        if (instruction != INST_FETCH_STOP) {
            PC++;
        }
    }
}

// CPU registers
typedef struct {
    word_t A;
    word_t D;
    addr_t PC;
} regs_s;
static regs_s Regs;

static addr_t pc_of_cycle_start; // only for debug
static word_t alu_out = 0;
static ap_uint<1> write_out = 0;
static word_t outM = 0;
static addr_t addressM = 0;
static word_t instruction;
static word_t first_inst; // only for debug
static word_t next_inst;
static ap_uint<1> cinst_phase = 0;
static uint64_t cycle = 0;
static uint64_t cycle_to_stop = 20; //0xFFFFFFFFFFFFFFFFull;
static word_t break_condition_bitmap = 0;

// CPU function
// ToDo: c-inst dual issue, dynamic dual issue mode, Xrom burst fetch
//       Separate M load 
static void cpu(word_t i_ram[IRAM_SIZE],
         word_t d_ram[DRAM_SIZE],
         ap_uint<1>& reset,
         hls::stream<unsigned int> uart_regs[4]) {
#pragma HLS INTERFACE ap_memory port=i_ram storage_type=ram_2p
#pragma HLS INTERFACE ap_memory port=d_ram storage_type=ram_t2p
#pragma HLS INTERFACE ap_none port=reset
#pragma HLS INTERFACE m_axi port=uart_regs depth=32

	bool break_condition = false;
    if (reset) {
        Regs.A = 0;
        Regs.D = 0;
        Regs.PC = 0;
        cycle = 0;
        write_out = 0;
        return;
    } else {
        for (; cycle < cycle_to_stop && !break_condition; cycle++) {
            #ifdef PIPELINE_II_1
            #pragma HLS PIPELINE II=1
            #else
            #pragma HLS PIPELINE II=2
            #endif

            //if (cycle == cycle_to_stop) break;
            pc_of_cycle_start = Regs.PC;
            write_out = 0;

            // Fetch
            instruction = i_ram[Regs.PC];
            first_inst = instruction; // only for debug
            #ifdef AINST_DUAL_ISSUE
            next_inst = i_ram[Regs.PC+1];
            #else
            next_inst = INST_NO_DUAL;
            #endif
            // Decode and execute instruction
            bool is_a_instruction = (instruction[15] == 0);
            
            if (is_a_instruction) {
                Regs.A = instruction;
                #ifdef AINST_DUAL_ISSUE
                instruction = next_inst;
                #endif
                Regs.PC++;
            }
            #ifdef AINST_DUAL_ISSUE
            else { next_inst = INST_NO_DUAL;}
            #else
            else
            #endif
            {
#if defined(PIPELINE_II_1) && defined(REDUCE_CINST_CYCLE)
                if ((cinst_phase == 0) && (instruction[12] == 0) && (instruction[3] == 0))
                {
                    // No memomory access case for both of source and detination, it can go through cinst_phase 1.
                    comp(instruction, Regs.A, Regs.D, alu_out);
                    cinst_phase = 1;
                }
#endif
#ifdef PIPELINE_II_1
                if (cinst_phase == 0) {
#endif
#if defined(PIPELINE_II_1) && defined(REDUCE_CINST_CYCLE)
                    // ALU computation using M
                    compM(instruction, Regs.A, Regs.D, d_ram, alu_out);
#else
                    // ALU computation using 
                    if (instruction[12] == 0) {
                        comp(instruction, Regs.A, Regs.D, alu_out);
                    } else {
                        compM(instruction, Regs.A, Regs.D, d_ram, alu_out);
                    }
#endif
                    cinst_phase = 1;
#ifdef PIPELINE_II_1
                } else {
#endif             
                    bool jump = false;
                    // Destination
                    addressM = Regs.A;
                    get_destination(instruction, alu_out,
                        Regs.A, Regs.D, d_ram, write_out, outM, addressM);
                    if (write_out && (addressM >= 0x4000)) {
                        char send_chars[10];
                        unsigned short addr = addressM - 0x4000;
                        send_chars[0] = '!';
                        for (int i = 1; i <= 4; i++) {
                            char temp = ((addr >> (4-i)*4) & 0xF);
                            send_chars[i] = (temp <= 9)? temp + '0': temp-10 + 'A';
                        }
                        for (int i = 5; i <= 8; i++) {
                            char temp = ((outM >> (8-i)*4) & 0xF);
                            send_chars[i] = (temp <= 9)? temp + '0': temp-10 + 'A';
                        }
                        send_chars[9] = '\n';
                        for (int i = 0; i < sizeof(send_chars); i++) {
                            while (uart_regs[1].full()) {}
                            uart_regs[1].write(send_chars[i]);
                        }
                        if (break_condition_bitmap | BREAK_CONDITION_BIT_DISPOUT) {
                            break_condition = true;
                        }
                    }

                    // Jump condition
                    jump = get_jump_condition(alu_out, instruction);

                    // Update PC
                    updatePC(jump, Regs.PC, Regs.A, instruction);                
                    cinst_phase = 0;
#ifdef PIPELINE_II_1
                }
#endif
            }
        }
        if (break_condition) cycle++;
    }
}

void cpu_wrapper(hls::stream<word_t>& command_packet_in,
                 hls::stream<word_t>& command_packet_out,
                 hls::stream<unsigned int> uart_regs[4]) {
                    
    #pragma HLS INTERFACE axis port=command_packet_in depth=32
    #pragma HLS INTERFACE axis port=command_packet_out depth=32
    //#pragma HLS INTERFACE ap_fifo port=debug 
    #pragma HLS INTERFACE m_axi port=uart_regs depth=32

    // Internal ROM
    static word_t i_ram[IRAM_SIZE];
    #pragma HLS BIND_STORAGE variable=i_ram type=RAM_2P impl=BRAM

    // Internal RAM
    static word_t d_ram[DRAM_SIZE];
    #pragma HLS BIND_STORAGE variable=d_ram type=RAM_T2P impl=BRAM

    static ap_uint<1> reset = 1;
    static ap_uint<1> halt = 1;

    while (!command_packet_in.empty()) {
        control_command_e command = (control_command_e)command_packet_in.read().to_int();
        switch(command) {
            case NORMAL_OPERATION:
                cycle_to_stop = 0xFFFFFFFFFFFFFFFFull;
                break;
            case SET_RESET_CONFIG:
            {
                word_t bitmap = command_packet_in.read();
                reset = (bitmap & RESET_BIT_RESET)? 1: 0;
                halt = (bitmap & RESET_BIT_HALT)? 1: 0;
                break;
            }
            case GET_RESET_CONFIG:
            {
                word_t bitmap = 0;
                if (reset) bitmap |= RESET_BIT_RESET;
                if (halt) bitmap |= RESET_BIT_HALT;
                command_packet_out.write(bitmap);
                break;
            }
            case WRITE_TO_IRAM: 
            {
                addr_t address = (addr_t)command_packet_in.read();
                word_t data = command_packet_in.read();
                i_ram[address] = data;
                break;
            }
            case LOAD_TO_IRAM: 
            {
                addr_t address = (addr_t)command_packet_in.read();
                word_t length = command_packet_in.read();
                for (word_t i = 0; i < length; i++) {
                    word_t data = command_packet_in.read();
                    i_ram[address+i] = data;
                }
                break;
            }
            case WRITE_TO_DRAM: 
            {
                addr_t address = (addr_t)command_packet_in.read();
                word_t data = command_packet_in.read();
                d_ram[address] = data;
                break;
            }
            case READ_FROM_DRAM: 
            {
                addr_t address = (addr_t)command_packet_in.read();
                command_packet_out.write(d_ram[address]);
                break;
            }
            case STEP_EXECUTION:
                cycle_to_stop = cycle+1;
                halt = 0;
                break;
            case SET_BREAK_CONDITION:
                break_condition_bitmap = command_packet_in.read();
                break;
            case GET_DEBUG_INFO:
            {
                word_t bitmap = command_packet_in.read();
                if (bitmap & DINFO_BIT_CYCLE) {
                    command_packet_out.write(cycle & 0xFFFF);
                    command_packet_out.write((cycle >> 16) & 0xFFFF);
                    command_packet_out.write((cycle >> 32) & 0xFFFF);
                    command_packet_out.write((cycle >> 48) & 0xFFFF);
                }
                if (bitmap & DINFO_BIT_WOUT) command_packet_out.write(write_out);
                if (bitmap & DINFO_BIT_OUTM) command_packet_out.write(outM);
                if (bitmap & DINFO_BIT_ADDRM) command_packet_out.write(addressM);
                if (bitmap & DINFO_BIT_PC) command_packet_out.write(pc_of_cycle_start);
                if (bitmap & DINFO_BIT_REGA) command_packet_out.write(Regs.A);
                if (bitmap & DINFO_BIT_REGD) command_packet_out.write(Regs.D);
                if (bitmap & DINFO_BIT_ALUO) command_packet_out.write(alu_out);
                if (bitmap & DINFO_BIT_INST1) command_packet_out.write(first_inst);
                if (bitmap & DINFO_BIT_INST2) command_packet_out.write(next_inst);
                if (bitmap & DINFO_BIT_SP) command_packet_out.write(d_ram[0]);
                break;
            }

            default:
                break;
        }
        if (reset || !halt) {
            cpu(i_ram, d_ram, reset, uart_regs);
        }
    }
}
