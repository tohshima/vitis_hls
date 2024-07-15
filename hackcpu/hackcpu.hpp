#include <ap_int.h>
#include <cstdint>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

// A-instruction fall through mode (can be dual issue with the next instruction)
#define AINST_FALL_THROUGH

// Define bit widths
const int WORD_WIDTH = 16;
const int ADDR_WIDTH = 15;

const int TRARCE_SIZE = 32;

// Debug instrction
#define INST_FETCH_STOP 0x8000
#define INST_NO_DUAL    0x8001

// Define CPU components
typedef ap_uint<WORD_WIDTH> word_t;
typedef ap_uint<ADDR_WIDTH> addr_t;

// Conntrol command
typedef enum {
    NORMAL_OPERATION    = 0x0000,
    SET_RESET           = 0x0001,
    CLEAR_RESET         = 0x0002,    
    WRITE_TO_IRAM       = 0x0010,
    WRITE_TO_DRAM       = 0x0012,
    READ_FROM_DRAM      = 0x0013,
    DUMP_FROM_DRAM      = 0x0014,
    STEP_EXECUTION      = 0x8000,
    GET_DEBUG_INFO      = 0x8010,
} control_command_e;

// For debug
typedef struct {
    uint32_t    cycle;
    ap_uint<1>  write_out;
    word_t      outM;
    addr_t      addressM;
    addr_t      pc;
    word_t      regA;
    word_t      regD;
    word_t      alu_out;
    word_t      instruction;
} debug_s;

typedef hls::axis<debug_s, 0,0,0> debug_t;

void cpu_wrapper(hls::stream<word_t>& command_packet_in,
                            hls::stream<word_t>& command_packet_out);


