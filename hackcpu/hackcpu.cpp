#include <string.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"
#include "peripheral_task.hpp"

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

static void compM(
    word_t instruction, 
    word_t A, 
    word_t D, 
    word_t ram[IRAM_SIZE],
    word_t &alu_out
) {
    #pragma HLS inline
    
    word_t x = D;
    word_t y = ram[A];
    comp_core(instruction, x, y, alu_out);
}

static void get_destination(
    word_t instruction, 
    word_t alu_out,
    word_t& A, 
    word_t& D,
    word_t ram[DRAM_SIZE], 
    ap_uint<1>& write_out, 
    word_t& outM, 
    addr_t addressM,
    hls::stream<addr_t>& periheral_waddr_out,
    hls::stream<word_t>& periheral_wdata_out
) {
#pragma HLS inline
    if (instruction[3]) {
        outM = alu_out;
        if (addressM >= PERIPHERAL_START_ADDR) {
            periheral_waddr_out.write(addressM);
            periheral_wdata_out.write(outM);
            write_out = 0;
        }
        ram[addressM] = outM;
        write_out = 1;
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

static bool updatePC(bool jump, addr_t& PC, word_t& A,
                     word_t instruction) {
#pragma HLS inline
	bool break_cond = false;
    if (jump) {
        PC = A;
    } else {
        if (instruction == INST_FETCH_STOP) {
        	break_cond = true;
        } else {
            PC++;
        }
    }
    return break_cond;
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
static word_t break_condition_bitmap = 0; // BREAK_CONDITION_BIT_DISPOUT | BREAK_CONDITION_BIT_KEYIN; // obsolete
static uint64_t break_cycle_interval = 0;

// CPU function
// ToDo: c-inst dual issue, dynamic dual issue mode, Xrom burst fetch
//       Separate M load 
word_t cpu(
	word_t i_ram[IRAM_SIZE],
	word_t d_ram[DRAM_SIZE],
	ap_uint<1>& reset,
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out
) {
    #pragma HLS INTERFACE ap_memory port=i_ram storage_type=ram_2p
    #pragma HLS INTERFACE ap_memory port=d_ram storage_type=ram_t2p
    #pragma HLS INTERFACE ap_none port=reset
	#pragma HLS INTERFACE axis port=interrupt_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=1

	word_t break_reason = BREAK_REASON_CYCLE;
    if (reset) {
        Regs.A = 0;
        Regs.D = 0;
        Regs.PC = 0;
        cycle = 0;
        write_out = 0;
        return BREAK_REASON_RESET;
    } else {
        for (; cycle < cycle_to_stop && (break_reason == BREAK_REASON_CYCLE); cycle++) {
            #ifdef PIPELINE_II_1
            #pragma HLS PIPELINE II=1
            #else
            #pragma HLS PIPELINE II=2
            #endif
            //#pragma HLS DATAFLOW

            //if (cycle == cycle_to_stop) break;
            pc_of_cycle_start = Regs.PC;
            write_out = 0;

            // interrupt check
            if (!interrupt_in.empty()) {
                // currently intterupt is used only to break for debugging
                word_t val = interrupt_in.read();
                switch (val & INT_REASON_MASK) {
                case INT_REASON_KEYIN:
                    d_ram[PERIPHERAL_KEYIN_ADDR] = val & 0xFF;
                    break;
                default:
                    break_reason = BREAK_REASON_EXT;
                    break;
                }
            }

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
                    // No memory access case for both of source and destination, it can go through cinst_phase 1.
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
                        Regs.A, Regs.D, d_ram, write_out, outM, addressM,
                        peripheral_waddr_out, peripheral_wdata_out);
                    //disp_out_or_key_in_check(write_out, addressM, break_condition_bitmap,
                    //		instruction[12], break_reason);

                    // Jump condition
                    jump = get_jump_condition(alu_out, instruction);

                    // Update PC
                    if (updatePC(jump, Regs.PC, Regs.A, instruction)) {
                    	break_reason = BREAK_REASON_STOP;
                    }
                    cinst_phase = 0;
#ifdef PIPELINE_II_1
                }
#endif
            }
        }
        if (break_reason != BREAK_REASON_CYCLE) {
        	cycle++;
        }
    }
    return break_reason;
}

#define SEND_NUM_RETVALS(num) {command_packet_out.write(num);}
#define SEND_STATUS(sta) {command_packet_out.write(sta);}

static word_t bit_count(word_t bitmap) {
	word_t count = 0;
	for (int i = 0; i < bitmap.width; i++) {
		if (bitmap[i]) count++;
	}
	return count;
}

void cpu_wrapper(
	hls::stream<word_t>& command_packet_in,
    hls::stream<word_t>& command_packet_out,
    hls::stream<word_t>& interrupt_in,
    hls::stream<addr_t>& peripheral_raddr_out,
    hls::stream<word_t>& peripheral_rdata_in,
    hls::stream<addr_t>& peripheral_waddr_out,
    hls::stream<word_t>& peripheral_wdata_out
) {
    #pragma HLS INTERFACE axis port=command_packet_in depth=32
    #pragma HLS INTERFACE axis port=command_packet_out depth=32
	#pragma HLS INTERFACE axis port=interrupt_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_raddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_rdata_in depth=1
	#pragma HLS INTERFACE axis port=peripheral_waddr_out depth=1
	#pragma HLS INTERFACE axis port=peripheral_wdata_out depth=1
    //#pragma HLS INTERFACE ap_fifo port=debug 

	// Internal ROM
	static word_t i_ram[IRAM_SIZE];
	#pragma HLS BIND_STORAGE variable=i_ram type=RAM_2P impl=BRAM

	// Internal RAM
	static word_t d_ram[DRAM_SIZE];
	#pragma HLS BIND_STORAGE variable=d_ram type=RAM_T2P impl=BRAM

	static ap_uint<1> reset = 1;
	static ap_uint<1> halt = 1;

	//while (!command_packet_in.empty()) {
		// feed a new command
		control_command_e command = (control_command_e)command_packet_in.read().to_int();
		switch(command) {
			case NORMAL_OPERATION:
				cycle_to_stop =  (break_cycle_interval > 0)? cycle+break_cycle_interval:  0xFFFFFFFFFFFFFFFFull;
				halt = 0;
				break;
			case SET_RESET_CONFIG:
			{
				while(command_packet_in.empty()) {}
				word_t bitmap = command_packet_in.read();
				reset = (bitmap & RESET_BIT_RESET)? 1: 0;
				halt = (bitmap & RESET_BIT_HALT)? 1: 0;
				SEND_NUM_RETVALS(0);
				break;
			}
			case GET_RESET_CONFIG:
			{
				word_t bitmap = 0;
				if (reset) bitmap |= RESET_BIT_RESET;
				if (halt) bitmap |= RESET_BIT_HALT;
				SEND_NUM_RETVALS(1);
				command_packet_out.write(bitmap);
				break;
			}
			case WRITE_TO_IRAM:
			{
				while(command_packet_in.empty()) {}
				addr_t address = (addr_t)command_packet_in.read();
				while(command_packet_in.empty()) {}
				word_t data = command_packet_in.read();
				i_ram[address] = data;
				SEND_NUM_RETVALS(0);
				break;
			}
			case LOAD_TO_IRAM:
			{
				while(command_packet_in.empty()) {}
				addr_t address = (addr_t)command_packet_in.read();
				while(command_packet_in.empty()) {}
				word_t length = command_packet_in.read();
				for (word_t i = 0; i < length; i++) {
					while(command_packet_in.empty()) {}
					word_t data = command_packet_in.read();
					i_ram[address+i] = data;
				}
				SEND_NUM_RETVALS(0);
				break;
			}
			case READ_FROM_IRAM:
			{
				while(command_packet_in.empty()) {}
				addr_t address = (addr_t)command_packet_in.read();
				SEND_NUM_RETVALS(1);
				command_packet_out.write(i_ram[address]);
				break;
			}
			case WRITE_TO_DRAM:
			{
				while(command_packet_in.empty()) {}
				addr_t address = (addr_t)command_packet_in.read();
				while(command_packet_in.empty()) {}
				word_t data = command_packet_in.read();
				d_ram[address] = data;
				SEND_NUM_RETVALS(0);
				break;
			}
			case READ_FROM_DRAM:
			{
				while(command_packet_in.empty()) {}
				addr_t address = (addr_t)command_packet_in.read();
				SEND_NUM_RETVALS(1);
				command_packet_out.write(d_ram[address]);
				break;
			}
			case STEP_EXECUTION:
				cycle_to_stop = cycle+1;
				halt = 0;
				break_cycle_interval = 0;
				break_condition_bitmap &= ~BREAK_CONDITION_BIT_INTERVAL;
				break;
			case SET_BREAK_CONDITION:
				while(command_packet_in.empty()) {}
				break_condition_bitmap = command_packet_in.read();
				if (break_condition_bitmap & BREAK_CONDITION_BIT_INTERVAL) {
					break_cycle_interval = command_packet_in.read().to_uint64();
				}
				SEND_NUM_RETVALS(0);
				break;
			case MULTI_STEP_EXECUTION:
			{
				while(command_packet_in.empty()) {}
				word_t steps = command_packet_in.read();
				cycle_to_stop = cycle+steps;
				halt = 0;
				SEND_NUM_RETVALS(0);
				break;
			}
			case GET_DEBUG_INFO:
			{
				while(command_packet_in.empty()) {}
				word_t bitmap = command_packet_in.read();
				word_t bitcnt = ((bitmap & DINFO_BIT_CYCLE) ? 3:0) + bit_count(bitmap);
				SEND_NUM_RETVALS(bitcnt);

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
				halt = 1;
				SEND_NUM_RETVALS(0);
				break;
		}
		word_t break_reason = BREAK_REASON_NOP;
		if (reset || !halt) {
			break_reason = cpu(i_ram, d_ram, reset, interrupt_in, peripheral_raddr_out, peripheral_rdata_in, peripheral_waddr_out, peripheral_wdata_out);
			if ((break_condition_bitmap & BREAK_CONDITION_BIT_INTERVAL) &&
					(break_reason == BREAK_REASON_CYCLE)) {
				break_reason = BREAK_REASON_INTERVAL;
			}
            if (reset) {
                for (int i = 0; i < sizeof(d_ram)/sizeof(word_t); i++) d_ram[i] = 0;
            } else {
                SEND_NUM_RETVALS(0);
            }
			halt = 1;
		}
		SEND_STATUS(break_reason);
	//}
}


