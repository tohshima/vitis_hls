// Hack CPU public definition  
#ifndef __HACKCPU_DEF_HPP__
#define __HACKCPU_DEF_HPP__

// Debug instruction
#define INST_FETCH_STOP 0x8000
#define INST_NO_DUAL    0x8001

// Break reason
typedef enum {
	BREAK_REASON_NOP       = 0x8800,
	BREAK_REASON_RESET     = 0x8801,
	BREAK_REASON_CYCLE     = 0x8802,
	BREAK_REASON_STOP      = 0x8804,
	BREAK_REASON_DISP      = 0x8808, // obsolete
	BREAK_REASON_INTERVAL  = 0x8810, // obsolete
	BREAK_REASON_KEYIN     = 0x8820, // obsolete
	BREAK_REASON_EXT       = 0x8880, // External signal
} break_reason_e;

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
    MULTI_STEP_EXECUTION= 0x8002,
    GET_DEBUG_INFO      = 0x8010,
} control_command_e;

typedef enum {
    RESET_BIT_RESET = 0x0001,
    RESET_BIT_HALT  = 0x0002,
} reset_config_bitmap_e;

typedef enum {
	BREAK_CONDITION_BIT_DISPOUT  = 0x0001, // obsolete
	BREAK_CONDITION_BIT_INTERVAL = 0x0002, // obsolete
	BREAK_CONDITION_BIT_KEYIN    = 0x0004, // obsolete
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

#endif // #ifndef __HACKCPU_DEF_HPP__
