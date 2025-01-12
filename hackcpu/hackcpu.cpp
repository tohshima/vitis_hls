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
#pragma HLS INTERFACE m_axi port=uart_regs depth=32

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
            	cycle_to_stop =  (break_cycle_interval > 0)? cycle+break_cycle_interval:  0xFFFFFFFFFFFFFFFFull;
                halt = 0;
                SEND_NUM_RETVALS(0);
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
                SEND_NUM_RETVALS(0);
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
            break_reason = cpu(i_ram, d_ram, reset);
            if ((break_condition_bitmap & BREAK_CONDITION_BIT_INTERVAL) &&
            		(break_reason == BREAK_REASON_CYCLE)) {
            	break_reason = BREAK_REASON_INTERVAL;
            }
            halt = 1;
        }
        command_packet_out.write(break_reason);
        if ((break_condition_bitmap & BREAK_CONDITION_BIT_DISPOUT) && (break_reason == BREAK_REASON_DISP)) {
        	command_packet_out.write(addressM);
        	command_packet_out.write(outM);
        }
    }
}

static word_t make_hex_bin(const char* hex_chars) {
	word_t hex_data = 0;
	for (int i = 0; i < 4; i++) {
		if ((hex_chars[i] >= '0') && (hex_chars[i] <= '9')) hex_data += ((hex_chars[i]-'0') << (3-i)*4);
		else if ((hex_chars[i] >= 'a') && (hex_chars[i] <= 'f')) hex_data += ((hex_chars[i]-'a'+10) << (3-i)*4);
		else if ((hex_chars[i] >= 'A') && (hex_chars[i] <= 'F')) hex_data += ((hex_chars[i]-'A'+10) << (3-i)*4);
	}
	return hex_data;
}

static word_t make_hex_chars(word_t hex_data, char* hex_chars) {
	for (int i = 0; i < 4; i++) {
		char temp = ((hex_data >> (3 - i) * 4) & 0xF);
		hex_chars[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'a';
	}
	hex_chars[4] = '\n';
	//hex_chars[5] = '\0';
	return 5;
}

static word_t make_disp_out(word_t addrM, word_t dataM, char* out) {
	static word_t last_data_ = 0;
	if (addrM >= 0x4000) {
		unsigned short addr = addrM - 0x4000;
		if (dataM == last_data_) {
			out[0] = '%';
			for (int i = 1; i <= 4; i++) {
				char temp = ((addr >> (4 - i) * 4) & 0xF);
				out[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
			}
			out[5] = '\n';
			//out[6] = '\0';
			return 6;
		} else {
			out[0] = '!';
			for (int i = 1; i <= 4; i++) {
				char temp = ((addr >> (4 - i) * 4) & 0xF);
				out[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
			}
			for (int i = 5; i <= 8; i++) {
				char temp = ((dataM >> (8 - i) * 4) & 0xF);
				out[i] = (temp <= 9) ? temp + '0' : temp - 10 + 'A';
			}
			out[9] = '\n';
			//out[10] = '\0';
			last_data_ = dataM;
			return 10;
		}
	}
	return 0;
}

static void cpu_wrapper2(word_t command, bool command_execute,
		word_t read_data[16], word_t& num_ret, word_t& ret_status,
		word_t disp_out[2], bool auto_continue, bool key_input, word_t key_code) {
    // CPU interface signals
    static hls::stream<word_t> command_in;
    static hls::stream<word_t> command_out;

    num_ret = 0;
	if (key_input) {
		command_in.write(WRITE_TO_DRAM);
		command_in.write(0x6000);
		command_in.write(key_code);
		cpu_wrapper(command_in, command_out);
		DUMMY_READ();
	}
	if (command_execute) {
		if (auto_continue) {
			// auto continue
			command_in.write(NORMAL_OPERATION);
		}
		// 実行
		cpu_wrapper(command_in, command_out);
		while (command_out.empty()) {}
		num_ret = command_out.read();
		for (int i = 0; i < num_ret; i++) {
			read_data[i] = command_out.read();
		}
		ret_status = command_out.read();
		if (ret_status == BREAK_REASON_DISP) {
			disp_out[0] = command_out.read();
			disp_out[1] = command_out.read();
		}
	} else {
		//sprintf(buf, "snd: %04x", send_data);
		//std::cout << buf << std::endl;
		command_in.write(command);
	}
}

void uart_bridge(const char char_in[4], bool& auto_continue_requested,
		char char_out[108], word_t& num_char_out, word_t& num_disp_out, bool& key_requested) {
	bool command_execute = false;
	word_t read_data[16];
	word_t num_ret = 0;
	word_t ret_status = 0;
	static word_t disp_out[2];
	bool auto_continue = false;
	bool key_input = false;
	word_t key_code = 0;

	num_char_out = 0;
	num_disp_out = 0;
	auto_continue_requested = false;
	key_requested = false;

	if (char_in[0] == 'K') {
		key_input = true;
		command_execute = true;
		auto_continue = true;
		key_code = (char_in[1]-'0')*100+(char_in[2]-'0')*10+(char_in[3]-'0');
	}
	else if (char_in[0] == 'N') {
		command_execute = true;
		auto_continue = true;
	} else if (char_in[0] == 'Z') {
		command_execute = true;
	}
	word_t command = make_hex_bin(char_in);
	cpu_wrapper2(command, command_execute, read_data, num_ret, ret_status,
			disp_out, auto_continue, key_input, key_code);
	if (ret_status == BREAK_REASON_DISP) {
		num_disp_out = make_disp_out(disp_out[0], disp_out[1], &char_out[0]);
		auto_continue_requested = true;
	} else if (ret_status == BREAK_REASON_KEYIN) {
		key_requested = true;
	} else if (command_execute) {
		num_char_out += make_hex_chars(num_ret, &char_out[0]);
		for (int i = 0; i < num_ret; i++) {
			num_char_out += make_hex_chars(read_data[i], &char_out[(i+1)*5]);
		}
		num_char_out += make_hex_chars(ret_status, &char_out[(num_ret+1)*5]);
	}
}

static bool initialized = false;
static int phase = -1;

static void send_char(volatile unsigned int *uart_reg, const char c) {
	// TXFIFOが満杯でないか確認
	while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
	// データをTXFIFOに書き込む
	uart_reg[TX_FIFO_OFFSET] = c;
}

static bool send_str(volatile unsigned int *uart_reg, const char *s, int& p, int length) {
	if ((s[p] != 0) && (p < length)) {
		send_char(uart_reg, s[p]);
		p++;
	}
	if (p == length) {
		p = 0;
		return true;
	}
	return false;
}

static bool get_token(const ap_uint<1>& interrupt,
	    volatile unsigned int *uart_reg,
		volatile char& debug_rx_data_,
		char* rx_buf, int& rx_bufp
) {
	while (uart_reg[STAT_REG_OFFSET] & 0x00000001) {
		debug_rx_data_ = uart_reg[RX_FIFO_OFFSET];
		rx_buf[rx_bufp++] = debug_rx_data_;
		if (rx_bufp == TOKEN_SIZE) {
			rx_bufp = 0;
			return true;
		}
	}
	return false;
}

static void uart_execute(
		const ap_uint<1>& interrupt,
		volatile unsigned int *uart_reg,
		volatile char& num_disp_out_,
		volatile char& num_char_out_,
		char* char_out_,
		char commandin_available_,
		char* commandin_,
		char keyin_available_,
		char* keyin_,
		volatile char& debug_phase_,
		volatile char& debug_rx_data_) {
    static bool auto_cont_requested = false;
    static bool key_requested = false;
    static char char_out[108];
    static word_t num_char_out = 0;
    static word_t num_disp_out = 0;
    static const char no_keyin[TOKEN_SIZE] = {'K','0','0','0'};
    static const char auto_continue[TOKEN_SIZE] = {'N','0','0','0'};

    static char rx_buf[TOKEN_SIZE] = {0};
    static int rx_bufp = 0;
    static int out_p = 0;
    static int key_in_count = 0;
    num_disp_out_ = 0;
    num_char_out_ = 0;

	if (num_disp_out) {
		// display out
		debug_phase_ = phase = 0xE3;
		while (send_str(uart_reg, char_out, out_p, num_disp_out)) {}
		num_disp_out_ = num_disp_out;
		num_disp_out = 0; // 1文字ずつ出して出し切った
		memcpy(char_out_, char_out, num_disp_out_);
	} else if (num_char_out) {
		// command completed
		debug_phase_ = phase = 0xE2;
		while (send_str(uart_reg, char_out, out_p, num_char_out)) {}
		num_char_out_ = num_char_out;
		num_char_out = 0; // 1文字ずつ出して出し切った
		memcpy(char_out_, char_out, num_char_out_);
	} else if (key_requested) {
	    // Key 入力
		debug_phase_ = phase = 0xE1;
		const char* key_in = no_keyin;
		if (get_token(interrupt, uart_reg, debug_rx_data_, rx_buf, rx_bufp)) {
			key_in = rx_buf;
		} else if (keyin_available_) {
			key_in = keyin_;
		}
		uart_bridge(key_in, auto_cont_requested, char_out, num_char_out, num_disp_out, key_requested);
	} else if (auto_cont_requested) {
		debug_phase_ = phase = 0xEA;
		uart_bridge(auto_continue, auto_cont_requested, char_out, num_char_out, num_disp_out, key_requested);
	} else {
		char* command_in = NULL;
		if (get_token(interrupt, uart_reg, debug_rx_data_, rx_buf, rx_bufp)) {
			command_in = rx_buf;
		} else if (commandin_available_) {
			command_in = commandin_;
		}
		if (command_in) {
		    // RXFIFOからデータを読み取る
			debug_phase_ = phase = 0xE0;
			uart_bridge(command_in, auto_cont_requested, char_out, num_char_out, num_disp_out, key_requested);
		}
	}
}

void uart_if(
	bool start,
	const ap_uint<1>& interrupt,
	volatile unsigned int *uart_reg,
	volatile char& num_disp_out_,
	volatile char& num_char_out_,
	char* char_out_,
	volatile char commandin_available_,
	char* commandin_,
	volatile char keyin_available_,
	char* keyin_,
	volatile char& debug_phase_,
	volatile char& debug_rx_data_,
	char debug_injection
) {
	#pragma HLS INTERFACE ap_none port=start
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct bundle=AXIM depth=20 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE ap_none port=debug_phase_
	#pragma HLS INTERFACE ap_none port=debug_rx_data_
    #pragma HLS INTERFACE ap_none port=return

	// ボーレート設定（例：115200 bps）
	// 注: 実際のボーレート設定はUART Lite IPの設定に依存します

    debug_phase_ = phase = 0;
	debug_rx_data_ = 0;
	if (start && !initialized) {
		debug_phase_ = phase = 2;
		initialized = true;
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // ソフトウェアリセット
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // リセット解除
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX割り込みを有効化

	} else if (start && debug_injection) {

		// データをTXFIFOに書き込む
		send_char(uart_reg, debug_injection);
		debug_phase_ = phase = 10;
		// RXFIFOからデータを読み取る
		while((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 0) {}
		debug_rx_data_ = uart_reg[RX_FIFO_OFFSET];

	} else if (start) {
		debug_phase_ = phase = 6;
		uart_execute(interrupt, uart_reg,
				num_disp_out_, num_char_out_, char_out_,
				commandin_available_, commandin_,
				keyin_available_, keyin_,
				debug_phase_, debug_rx_data_);
    }
}
