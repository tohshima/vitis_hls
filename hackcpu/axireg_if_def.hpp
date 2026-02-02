// Hack CPU AXI register IF public definition
#ifndef __AXIREG_IF_DEF_HPP__
#define __AXIREG_IF_DEF_HPP__

#ifndef VITIS_HLS_SIM 
// for test bench  &  PS program
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
typedef int bool;
#define false (0)
#define true (1)
#endif
typedef uint16_t hackcpu_reg_t;

#else
// vitis hls environment
#define AXI_REG_WIDTH 16
typedef ap_uint<AXI_REG_WIDTH> hackcpu_reg_t;

#endif

typedef struct {
    hackcpu_reg_t uart_control;
    hackcpu_reg_t command_control;
    hackcpu_reg_t command_status;
    hackcpu_reg_t command_in_word;
    hackcpu_reg_t command_in_num_params;
    hackcpu_reg_t command_in_params[18];
    hackcpu_reg_t command_out_status;
    hackcpu_reg_t command_out_num_params;
    hackcpu_reg_t command_out_params[16];
} hackcpu_regs_t __attribute__((__packed__)) __attribute__((aligned(2)));

typedef enum{
    UART_CONTROL_IF_ENABLE_BIT  = 0,
    UART_CONTROL_DISP_OUT_BIT   = 1
} uart_control_bit_e;

typedef enum {
    CMD_CTRL_START_BIT = 0
} cmd_ctrl_bit_e;

typedef enum {
    CMD_STAT_BUSY_BIT = 0,
    CMD_STAT_DONE_BIT = 1
} cmd_stat_bit_e;

static inline void axireg_set_uart_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control |= (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_clear_uart_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control &= ~(1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline bool axireg_is_uart_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->uart_control & (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_set_uart_disp_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control |= (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline void axireg_clear_uart_disp_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control &= ~(1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_is_uart_disp_enable(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->uart_control & (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_start_command(hackcpu_regs_t* p_reg, hackcpu_reg_t word, hackcpu_reg_t length, const hackcpu_reg_t params[]) {
    #pragma HLS INLINE
    if (p_reg->command_status & (1 << CMD_STAT_BUSY_BIT)) return false;
    p_reg->command_status = 0;
    p_reg->command_in_word = word;
    p_reg->command_in_num_params = length;
    for (hackcpu_reg_t i = 0; i < length; i++) {
        p_reg->command_in_params[i] = params[i];
    }
    p_reg->command_control |= (1 << CMD_CTRL_START_BIT);
    return true;
}
static inline void axireg_set_command_start(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_control |= (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_clear_command_start(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_control &= ~(1 << CMD_CTRL_START_BIT);
}
static inline bool axireg_is_command_start(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_control & (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_set_command_busy(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status |= (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_clear_command_busy(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status &= ~(1 << CMD_STAT_BUSY_BIT);
}
static inline bool axireg_is_command_busy(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_status & (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_set_command_done(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status |= (1 << CMD_STAT_DONE_BIT);
}
static inline void axireg_clear_command_done(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status &= ~(1 << CMD_STAT_DONE_BIT);
}
static inline bool axireg_is_command_done(hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_status & (1 << CMD_STAT_DONE_BIT);
}
static inline hackcpu_reg_t axireg_get_command_result(hackcpu_regs_t* p_reg, hackcpu_reg_t params[]) {
    #pragma HLS INLINE
    hackcpu_reg_t length = p_reg->command_out_num_params;
    for (hackcpu_reg_t i = 0; i < length; i++) {
        params[i] = p_reg->command_out_params[i];
    }
    return p_reg->command_out_status;
}

#include <cstdlib>
#include <cstring>
#include <string>
#include "hackcpu_def.hpp" // Assuming the CPU function is in a file named cpu.h
#ifdef VITIS_HLS_SIM
#include <iostream>
#include <bitset>
#include <iomanip>
#include <ap_int.h>
#include "hackcpu_if.hpp"
#else
#include "xil_printf.h"
#endif
#include "axireg_if_def.hpp"
#include "rom_pong.h"
#include "test_bench_axireg.hpp"

static void _print(const char* buf) {
#ifdef VITIS_HLS_SIM
    std::cout << buf;
#else
    //print(buf);
#endif
}
static inline void _printline(const char* buf) {
#ifdef VITIS_HLS_SIM
    std::cout << buf << std::endl;
#else
    //print(buf);
    //print("¥n");
#endif
}

static inline void print_regs(hackcpu_regs_t* p_axi_regs, int& count) {
    char buf[256];
    ::sprintf(buf, "========== %d", count++);
    _printline(buf);
    ::sprintf(buf, "  0x00: UART_CONTROL   = 0x%04x", p_axi_regs->uart_control.to_ushort());
    _printline(buf);
    ::sprintf(buf, "  0x02: CMD_CONTROL    = 0x%04x, 0x04: CMD_STATUS     = 0x%04x", p_axi_regs->command_control.to_ushort(), p_axi_regs->command_status.to_ushort());
    _printline(buf);
    ::sprintf(buf, "  0x06: CMD_IN_WORD    = 0x%04x, 0x08: CMD_IN_#_PRMS  = 0x%04x", p_axi_regs->command_in_word.to_ushort(), p_axi_regs->command_in_num_params.to_ushort());
    _printline(buf);
    ::sprintf(buf, "  0x0a: CMD_IN_PARAMS  = 0x%04x", p_axi_regs->command_in_params[0].to_ushort());
    _print(buf);
    for (unsigned int i = 1; i < p_axi_regs->command_in_num_params; i++) {
        ::sprintf(buf, " 0x%04x", p_axi_regs->command_in_params[i].to_ushort());
        _print(buf);
    }
    _printline("");
    ::sprintf(buf, "  0x2e: CMD_OUT_STATUS = 0x%04x, 0x30: CMD_OUT_#_PRMS = 0x%04x", p_axi_regs->command_out_status.to_ushort(), p_axi_regs->command_out_num_params.to_ushort());
    _printline(buf);
    ::sprintf(buf, "  0x32: CMD_OUT_PARAMS = 0x%04x", p_axi_regs->command_out_params[0].to_ushort());
    _print(buf);
    for (hackcpu_reg_t i = 1; i < p_axi_regs->command_out_num_params; i++) {
        ::sprintf(buf, " 0x%04x", p_axi_regs->command_out_params[i].to_ushort());
        _print(buf);
    }
    _printline("");
    _printline("");
}

static inline void execute_commannd(hackcpu_regs_t* p_reg, hackcpu_reg_t word, hackcpu_reg_t length, const hackcpu_reg_t* params, int& reg_count) {
    axireg_start_command(p_reg, word, length, params);
    do {
#ifdef VITIS_HLS_SIM
        extern int hackcpu_if(
            hackcpu_regs_t& axi_regs,
            hls::stream<addr_t>& dispadr_out_fw,
            hls::stream<word_t>& dispdat_out_fw,
            volatile ap_uint<1> button_in0,
            volatile ap_uint<1> button_in1,
            volatile ap_uint<1> button_in2,
            volatile ap_uint<1> button_in3,
            volatile ap_uint<1> btn_smp_clk,
            volatile ap_uint<1>& led_btn_L_out,
            volatile ap_uint<1>& led_btn_R_out,
            volatile ap_uint<1>& led_active_out,
            volatile unsigned int *uart_reg,
            volatile ap_uint<8>& debug_phase
        );
        volatile ap_uint<1> button_in0 = 0;
        volatile ap_uint<1> button_in1 = 0;
        volatile ap_uint<1> button_in2 = 0;
        volatile ap_uint<1> button_in3 = 0;
        volatile ap_uint<1> btn_smp_clk = 0;
        volatile ap_uint<1> led_btn_L_out = 0;
        volatile ap_uint<1> led_btn_R_out = 0;
        volatile ap_uint<1> led_active_out = 0;
        volatile ap_uint<8> debug_phase = 0;
        volatile unsigned int uart_reg[UART_REG_SIZE] = {0};
        hls::stream<addr_t> dispadr_out_fw;
        hls::stream<word_t> dispdat_out_fw;
        print_regs(p_reg, reg_count);
        hackcpu_if(
            *p_reg,
            dispadr_out_fw, dispdat_out_fw,
            button_in0, button_in1, button_in2, button_in3,
            btn_smp_clk, led_btn_L_out, led_btn_R_out, led_active_out,
            uart_reg, debug_phase);
#endif
        print_regs(p_reg, reg_count);
    } while (!axireg_is_command_done(p_reg));
}

static inline void config_reset(hackcpu_regs_t* p_reg, hackcpu_reg_t config, int& reg_count) {
    hackcpu_reg_t params[1];
    params[0] = config;
    execute_commannd(p_reg, SET_RESET_CONFIG, 1, params, reg_count);
}

static inline void load_rom(hackcpu_regs_t* p_reg, const hackcpu_reg_t* rom, int rom_length, int& reg_count) {
    const int one_length = 16;
    int curr_pointer = 0;
    int length = 0;
    do {
        length = (rom_length - curr_pointer) > one_length? one_length: (rom_length - curr_pointer);
        if (length > 0) {
            hackcpu_reg_t params[one_length+2];
            params[0] = curr_pointer;
            params[1] = length;
            for (int i = 0; i < length; i++) {
                params[2+i] = rom[curr_pointer++];
            }
            execute_commannd(p_reg, LOAD_TO_IRAM, length+2, params, reg_count);
        }
    } while (length > 0);
}

static inline void normal_operation(hackcpu_regs_t* p_reg, int& reg_count) {
    execute_commannd(p_reg, NORMAL_OPERATION, 0, NULL, reg_count);
}

static inline void test_bench_axireg(hackcpu_regs_t* p_reg) {

    //memset(p_reg, 0, sizeof(axi_regs_t));
    axireg_clear_uart_enable(p_reg);
    axireg_set_uart_disp_enable(p_reg);
    int reg_count = 0;

    // Reset
    config_reset(p_reg, RESET_BIT_RESET | RESET_BIT_HALT, reg_count);
    config_reset(p_reg, RESET_BIT_HALT, reg_count);

    load_rom(p_reg, pong_rom_code, sizeof(pong_rom_code)/sizeof(pong_rom_code[0]), reg_count);
    normal_operation(p_reg, reg_count);
}

#endif //  __AXIREG_IF_DEF_HPP__