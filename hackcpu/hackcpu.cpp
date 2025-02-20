#include <string.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"
#ifndef __SYNTHESIS__
#include "uart_comm.hpp"
#endif

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
static word_t break_condition_bitmap = BREAK_CONDITION_BIT_DISPOUT | BREAK_CONDITION_BIT_KEYIN;
static uint64_t break_cycle_interval = 0;

static void disp_out_or_key_in_check(ap_uint<1> write_out, addr_t addressM, word_t break_condition_bitmap,
		bool mem_access, word_t& break_reason) {
	if (break_condition_bitmap & BREAK_CONDITION_BIT_DISPOUT) {
		if (write_out && (addressM >= 0x4000)) {
			break_reason = BREAK_REASON_DISP;
		}
	}
	if (mem_access && (break_condition_bitmap & BREAK_CONDITION_BIT_KEYIN)) {
		if (addressM == 0x6000) {
			break_reason = BREAK_REASON_KEYIN;
		}
	}
}

// CPU function
// ToDo: c-inst dual issue, dynamic dual issue mode, Xrom burst fetch
//       Separate M load 
static word_t cpu(word_t i_ram[IRAM_SIZE],
         word_t d_ram[DRAM_SIZE],
         ap_uint<1>& reset) {
#pragma HLS INTERFACE ap_memory port=i_ram storage_type=ram_2p
#pragma HLS INTERFACE ap_memory port=d_ram storage_type=ram_t2p
#pragma HLS INTERFACE ap_none port=reset

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
                    disp_out_or_key_in_check(write_out, addressM, break_condition_bitmap,
                    		instruction[12], break_reason);

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

static word_t bit_count(word_t bitmap) {
	word_t count = 0;
	for (int i = 0; i < bitmap.width; i++) {
		if (bitmap[i]) count++;
	}
	return count;
}

void cpu_wrapper(hls::stream<word_t>& command_packet_in,
                 hls::stream<word_t>& command_packet_out) {
                    
    #pragma HLS INTERFACE axis port=command_packet_in depth=32
    #pragma HLS INTERFACE axis port=command_packet_out depth=32
    //#pragma HLS INTERFACE ap_fifo port=debug 

    // Internal ROM
    static word_t i_ram[IRAM_SIZE];
    #pragma HLS BIND_STORAGE variable=i_ram type=RAM_2P impl=BRAM

    // Internal RAM
    static word_t d_ram[DRAM_SIZE];
    #pragma HLS BIND_STORAGE variable=d_ram type=RAM_T2P impl=BRAM

    static ap_uint<1> reset = 1;
    static ap_uint<1> halt = 1;

    static int state = -1; // -1: wait for command receive 0: command received, 1: wait for args receive, 2: arg received
    static int num_args = 0;
    static control_command_e curr_commmand = NO_OPERATION;

	if (state == -1) {
		//if (!command_packet_in.empty()) {
			curr_commmand = (control_command_e)command_packet_in.read().to_int();
			state = 0;
		//}
	}
	if ((state == 0) || (state == 1)) {
		switch(curr_commmand) {
			case NORMAL_OPERATION:
				cycle_to_stop =  (break_cycle_interval > 0)? cycle+break_cycle_interval:  0xFFFFFFFFFFFFFFFFull;
				halt = 0;
				//SEND_NUM_RETVALS(0);
				state = 2;
				break;
			case SET_RESET_CONFIG:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					if (num_args /*&& !command_packet_in.empty()*/) {
						word_t bitmap = command_packet_in.read();
						reset = (bitmap & RESET_BIT_RESET)? 1: 0;
						halt = (bitmap & RESET_BIT_HALT)? 1: 0;
						SEND_NUM_RETVALS(0);
						state = 2;
					}
				}
				break;
			}
			case GET_RESET_CONFIG:
			{
				word_t bitmap = 0;
				if (reset) bitmap |= RESET_BIT_RESET;
				if (halt) bitmap |= RESET_BIT_HALT;
				SEND_NUM_RETVALS(1);
				state = 2;
				command_packet_out.write(bitmap);
				break;
			}
			case WRITE_TO_IRAM:
			{
				if (state == 0) {
					state = 1;
					num_args = 2;
				}
				if (state == 1) {
					static addr_t w2i_address;
					while (num_args /*&& !command_packet_in.empty()*/) {
						if (num_args == 2) {
							w2i_address = (addr_t)command_packet_in.read();
						} else if (num_args == 1) {
							word_t data = command_packet_in.read();
							i_ram[w2i_address] = data;
							SEND_NUM_RETVALS(0);
							state = 2;
						}
						num_args--;
					}
				}
				break;
			}
			case LOAD_TO_IRAM:
			{
				if (state == 0) {
					state = 1;
					num_args = 2;
				}
				if (state == 1) {
					static addr_t l2i_address = 0;
					static word_t l2i_length = 0;
					static bool l2i_got_length = false;
					while (num_args /*&& !command_packet_in.empty()*/) {
						if (!l2i_got_length) {
							if (num_args == 2) {
								l2i_address = (addr_t)command_packet_in.read();
							} else if (num_args == 1) {
								l2i_length = command_packet_in.read();
								num_args = l2i_length;
								l2i_got_length = true;
								continue;
							}
						} else {
							word_t data = command_packet_in.read();
							i_ram[l2i_address+l2i_length-num_args] = data;
						}
						num_args--;
					}
					if (num_args == 0) {
						SEND_NUM_RETVALS(0);
						l2i_got_length = false;
						state = 2;
					}
				}
				break;
			}
			case READ_FROM_IRAM:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					if (num_args /*&& !command_packet_in.empty()*/) {
						addr_t rfi_address = (addr_t)command_packet_in.read();
						SEND_NUM_RETVALS(1);
						command_packet_out.write(i_ram[rfi_address]);
						state = 2;
					}
				}
				break;
			}
			case WRITE_TO_DRAM:
			{
				if (state == 0) {
					state = 1;
					num_args = 2;
				}
				if (state == 1) {
					static addr_t w2d_address = 0;
					while (num_args /*&& !command_packet_in.empty()*/) {
						if (num_args == 2) {
							w2d_address = (addr_t)command_packet_in.read();
						} else if (num_args == 1) {
							word_t data = command_packet_in.read();
							d_ram[w2d_address] = data;
							SEND_NUM_RETVALS(0);
							state = 2;
						}
						num_args--;
					}
				}
				break;
			}
			case READ_FROM_DRAM:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					if (num_args /*&& !command_packet_in.empty()*/) {
						addr_t rfd_address = (addr_t)command_packet_in.read();
						SEND_NUM_RETVALS(1);
						command_packet_out.write(d_ram[rfd_address]);
						state = 2;
					}
				}
				break;
			}
			case STEP_EXECUTION:
				cycle_to_stop = cycle+1;
				halt = 0;
				break_cycle_interval = 0;
				break_condition_bitmap &= ~BREAK_CONDITION_BIT_INTERVAL;
				SEND_NUM_RETVALS(0);
				state = 2;
				break;
			case SET_BREAK_CONDITION:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					static bool got_bcb = false;
					if (!got_bcb /*&& !command_packet_in.empty()*/) {
						break_condition_bitmap = command_packet_in.read();
						got_bcb = true;
					}
					if (break_condition_bitmap & BREAK_CONDITION_BIT_INTERVAL) {
						if (got_bcb /*&& !command_packet_in.empty()*/) {
							break_cycle_interval = command_packet_in.read().to_uint64();
							SEND_NUM_RETVALS(0);
							got_bcb = false;
							state = 2;
						}
					} else {
						if (got_bcb) {
							SEND_NUM_RETVALS(0);
							got_bcb = false;
							state = 2;
						}
					}
				}
				break;
			}
			case MULTI_STEP_EXECUTION:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					if (num_args /*&& !command_packet_in.empty()*/) {
						word_t steps = command_packet_in.read();
						cycle_to_stop = cycle+steps;
						halt = 0;
						SEND_NUM_RETVALS(0);
						state = 2;
					}
				}
				break;
			}
			case GET_DEBUG_INFO:
			{
				if (state == 0) {
					state = 1;
					num_args = 1;
				}
				if (state == 1) {
					if (num_args /*&& !command_packet_in.empty()*/) {
						word_t bitmap = command_packet_in.read();
						word_t bitcnt = ((bitmap & DINFO_BIT_CYCLE) ? 3:0) + bit_count(bitmap);
						SEND_NUM_RETVALS(bitcnt);
						state = 2;

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
					}
				}
				break;
			}

			default:
				halt = 1;
				SEND_NUM_RETVALS(0);
				state = 2;
				num_args = 0;
				break;
		}
	}
	if (state == 2) {
		word_t break_reason = BREAK_REASON_NOP;
		if (reset || !halt) {
			break_reason = cpu(i_ram, d_ram, reset);
			if ((break_condition_bitmap & BREAK_CONDITION_BIT_INTERVAL) &&
					(break_reason == BREAK_REASON_CYCLE)) {
				break_reason = BREAK_REASON_INTERVAL;
			}
			halt = 1;
		}
		if ((break_condition_bitmap & BREAK_CONDITION_BIT_DISPOUT) && (break_reason == BREAK_REASON_DISP)) {
			SEND_NUM_RETVALS(0xFFFF);
			command_packet_out.write(addressM);
			command_packet_out.write(outM);
			curr_commmand = NORMAL_OPERATION;
			state = 0; // re-execute NOMAL_OPERATION for next call without response to host
		} else {
			if (curr_commmand == NORMAL_OPERATION) {
				SEND_NUM_RETVALS(0);
			}
			command_packet_out.write(break_reason);
			state = -1;
		}
	}
}


volatile char debug_phase_t1_ = 0;
volatile char debug_phase_t2_ = 0;
volatile char debug_phase_t3_ = 0;
volatile char debug_rx_data_ = 0;
volatile word_t debug_command_ = 0;

static char convert2hex(char c) {
	char h = 0;
	if ((c >= '0') && (c <= '9')) { h = (c-'0'); }
	else if ((c >= 'a') && (c <= 'f')) { h = (c-'a'+10); }
	else if ((c >= 'A') && (c <= 'F')) { h = (c-'A'+10); }
	return h;
}
static word_t make_hex_bin(uint32_t hex_chars4) {
	word_t hex_data = 0;
	for (int i = 0; i < 4; i++) {
		char c = (hex_chars4 >> i*8) & 0xFF;
		hex_data += convert2hex(c) << (3-i)*4;
	}
	return hex_data;
}

void convert_uart_to_command(
	hls::stream<token_word_t>& uart_in,
	hls::stream<word_t>& command_in
) {
	#pragma HLS INTERFACE axis port=uart_in depth=32
	#pragma HLS INTERFACE axis port=command_in depth=32

	static word_t read_data[16];
	#pragma HLS BIND_STORAGE variable=read_data type=RAM_T2P impl=BRAM

	bool key_input = false;
	word_t key_code = 0;
	static bool key_requested = false;

	debug_phase_t1_ = 0xB0;
	//if (!uart_in.empty()) {
		token_word_t token = uart_in.read();
		if ((token & 0xFF) == 'K') {
			debug_phase_t1_ = 0xB1;
			key_input = true;
			key_code = (((token >> 8)&0xFF)-'0')*100+(((token >> 16)&0xFF)-'0')*10+(((token >> 24)&0xFF)-'0'); // ToDo: conversion is not general
		}

		if (key_input) {
			debug_phase_t1_ = 0xB2;
			command_in.write(WRITE_TO_DRAM);
			command_in.write(0x6000);
			command_in.write(key_code);
			command_in.write(NORMAL_OPERATION);
		} else {
			word_t command = make_hex_bin(token);
			debug_command_ = command;
			command_in.write(command);
		}
	//}
}

void uart_in_task(
	hls::stream<token_word_t>& uart_in,
	hls::stream<word_t>& command_in
) {
	#pragma HLS INTERFACE axis port=uart_in depth=32
	#pragma HLS INTERFACE axis port=command_in depth=32

	convert_uart_to_command(uart_in, command_in);
}

void comp_task(
	hls::stream<word_t>& command_in,
	hls::stream<word_t>& command_out
) {
	#pragma HLS INTERFACE axis port=command_in depth=32
	#pragma HLS INTERFACE axis port=command_out depth=32

	cpu_wrapper(command_in, command_out);
}

static int make_hex_chars(
	word_t hex_data,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=128

	for (int i = 0; i < 4; i++) {
		char temp = ((hex_data >> (3 - i) * 4) & 0xF);
		char hex_char = (temp <= 9) ? temp + '0' : temp - 10 + 'a';
		uart_out.write(hex_char);
	}
	uart_out.write('\n');
	//hex_chars[5] = '\0';
	return 5;
}

static int make_disp_out(
	word_t addrM,
	word_t dataM,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=128

	static word_t last_data_ = 0;
	if (addrM >= 0x4000) {
		unsigned short addr = addrM - 0x4000;
#if 1
		if (dataM == last_data_) {
			uart_out.write('%');
			for (int i = 1; i <= 4; i++) {
				char temp = ((addr >> (4 - i) * 4) & 0xF);
				char out = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
				uart_out.write(out);
			}
			uart_out.write('\n');
			//out[6] = '\0';
			return 6;
		} else
#endif
		{
			uart_out.write('!');
			for (int i = 1; i <= 4; i++) {
				char temp = ((addr >> (4 - i) * 4) & 0xF);
				char out = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
				uart_out.write(out);
			}
			for (int i = 5; i <= 8; i++) {
				char temp = ((dataM >> (8 - i) * 4) & 0xF);
				char out = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
				uart_out.write(out);
			}
			uart_out.write('\n');
			//out[10] = '\0';
			last_data_ = dataM;
			return 10;
		}
	}
	return 0;
}

static int make_key_request(
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=uart_out depth=128

	uart_out.write('?');
	uart_out.write('\n');
	return 2;
}

void convert_out_to_uart(
	hls::stream<word_t>& command_out,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=command_out depth=32
	#pragma HLS INTERFACE axis port=uart_out depth=128

	//if (!command_out.empty()) {
		debug_phase_t3_ = 0xD0;

		word_t num_ret = command_out.read();
		if (num_ret != 0xFFFF) {
			make_hex_chars(num_ret, uart_out);

			debug_phase_t3_ = 0xD2;
			for (int i = 0; i < num_ret; i++) {
				word_t data = command_out.read();
				make_hex_chars(data, uart_out);
			}

			debug_phase_t3_ = 0xD3;
			word_t ret_status = command_out.read();
			make_hex_chars(ret_status, uart_out);

			if (ret_status == BREAK_REASON_KEYIN) {
				debug_phase_t3_ = 0xDE;
				make_key_request(uart_out);
			}
		}
		else /*if (ret_status == BREAK_REASON_DISP)*/ {
			debug_phase_t3_ = 0xDD;
			word_t addrM = command_out.read();
			word_t dataM = command_out.read();
			make_disp_out(addrM, dataM, uart_out);
		}
	//}
}

void uart_out_task(
	hls::stream<word_t>& command_out,
	hls::stream<char>& uart_out
) {
	#pragma HLS INTERFACE axis port=command_out depth=32
	#pragma HLS INTERFACE axis port=uart_out depth=128

	convert_out_to_uart(command_out, uart_out);
}

#ifndef __SYNTHESIS__
#define USE_COM "COM2"
uart_comm uart_comm("\\\\.\\" USE_COM);  // COM2ポートを開く
#endif

static void send_char(volatile unsigned int *uart_reg, const char c) {
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない

	// TXFIFOが満杯でないか確認
	while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
	// データをTXFIFOに書き込む
	uart_reg[TX_FIFO_OFFSET] = c;
}

static void send_chars(volatile unsigned int *uart_reg, hls::stream<char>& uart_out) {
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE axis port=uart_out depth=128

#ifndef __SYNTHESIS__
	char uo[128];
	DWORD length = 0;
	for (int i = 0; i < sizeof(uo); i++) {
		if (uart_out.empty()) break;
		uo[i] = uart_out.read();
		length++;
	}
	if (length) {
		uart_comm.write_data(uo, length);
	}
#else
	while (!uart_out.empty()) {
		send_char(uart_reg, uart_out.read());
	}
#endif
}

static bool get_token(
	    volatile unsigned int *uart_reg,
		hls::stream<ap_uint<8*TOKEN_SIZE>>& uart_in
) {
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE axis port=uart_in depth=32

	static int char_index = 0;
	static ap_uint<8*TOKEN_SIZE> token = 0;
	static bool key_in_flag = false;

#ifndef __SYNTHESIS__
    char read_buf[1];
    DWORD bytes_read = 0;
	while ((uart_comm.read_data(read_buf, sizeof(read_buf), bytes_read)) &&
						(bytes_read == sizeof(read_buf))) {
		debug_rx_data_ = read_buf[0];
#else
	if ((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 1) {
		debug_rx_data_ = uart_reg[RX_FIFO_OFFSET];
#endif
		token |= (debug_rx_data_ << 8*char_index);
		char_index++;
		if (char_index == TOKEN_SIZE) {
			uart_in.write(token);
			char_index = 0;
			token = 0;
			return true;
		}
	}
	return false;
}

static bool initialized = false;
//static int phase = -1;

void uart_if(
	bool start,
	volatile unsigned int *uart_reg,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
	volatile char& debug_phase__,
	volatile char& debug_rx_data__
) {
	#pragma HLS INTERFACE ap_none port=start
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
    #pragma HLS INTERFACE ap_none port=return

	// ボーレート設定（例：115200 bps）
	// 注: 実際のボーレート設定はUART Lite IPの設定に依存します

    debug_phase__ = 0;
	debug_rx_data__ = debug_rx_data_ = 0;
	if (start && !initialized) {
		debug_phase__ = 2;
		initialized = true;
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // ソフトウェアリセット
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // リセット解除
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX割り込みを有効化

	} else if (start) {
		//#pragma HLS DATAFLOW

		//while (1) {
			debug_phase__ = 6;

			get_token(uart_reg, uart_in);
			debug_phase__ = 10;
			send_chars(uart_reg, uart_out);
		//}
    }
}
