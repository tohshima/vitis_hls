#include <ap_int.h>
#include <cstdint>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

// A-instruction fall through mode (can be dual issue with the next instruction)
#define AINST_DUAL_ISSUE

//#define PIPELINE_II_1
//#define REDUCE_CINST_CYCLE

// Define bit widths
const int WORD_WIDTH = 16;
const int ADDR_WIDTH = 15;
const int IRAM_SIZE  = 1 << ADDR_WIDTH;
const int DRAM_SIZE  = 1 << ADDR_WIDTH;
const int TRARCE_SIZE = 32;

// Debug instruction
#define INST_FETCH_STOP 0x8000
#define INST_NO_DUAL    0x8001

// Define CPU components
typedef ap_uint<WORD_WIDTH> word_t;
typedef ap_int<WORD_WIDTH> sword_t;
typedef ap_uint<ADDR_WIDTH> addr_t;

// Conntrol command
typedef enum {
    NO_OPERATION        = 0x0000,
    NORMAL_OPERATION    = 0x0001,
    SET_RESET_CONFIG    = 0x0002,
    GET_RESET_CONFIG    = 0x0003,    
    WRITE_TO_IRAM       = 0x0010,
    LOAD_TO_IRAM        = 0x0011,
    READ_FROM_IRAM      = 0x0012,
    WRITE_TO_DRAM       = 0x0020,
    READ_FROM_DRAM      = 0x0021,
    DUMP_FROM_DRAM      = 0x0022,
    STEP_EXECUTION      = 0x8000,
    SET_BREAK_CONDITION = 0x8001,
    GET_DEBUG_INFO      = 0x8010,
} control_command_e;

typedef enum {
    RESET_BIT_RESET = 0x0001,
    RESET_BIT_HALT  = 0x0002,
} reset_config_bitmap_e;

typedef enum {
	BREAK_CONDITION_BIT_DISPOUT = 0x0001,
} break_condition_bitmap_e;

typedef enum {
    DINFO_BIT_CYCLE     = 0x0001,
    DINFO_BIT_WOUT      = 0x0002,
    DINFO_BIT_OUTM      = 0x0004,
    DINFO_BIT_ADDRM     = 0x0008,
    DINFO_BIT_PC        = 0x0010,
    DINFO_BIT_REGA      = 0x0020,
    DINFO_BIT_REGD      = 0x0040,
    DINFO_BIT_ALUO      = 0x0080,
    DINFO_BIT_INST1     = 0x0100,
    DINFO_BIT_INST2     = 0x0200,
	DINFO_BIT_SP		= 0x0400,
} debug_info_bitmap_e;

// For debug
typedef struct {
    uint64_t    cycle;
    ap_uint<1>  write_out;
    word_t      outM;
    addr_t      addressM;
    addr_t      pc;
    word_t      regA;
    word_t      regD;
    word_t      alu_out;
    word_t      instruction1;
    word_t      instruction2;
    word_t		sp;
} debug_s;

void cpu_wrapper(hls::stream<word_t>& command_packet_in,
                 hls::stream<word_t>& command_packet_out,
                 hls::stream<unsigned int> uart_regs[4]);


