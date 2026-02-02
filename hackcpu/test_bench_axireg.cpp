// test bench for hackcpu
#if 0

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
static void _printline(const char* buf) {
#ifdef VITIS_HLS_SIM
    std::cout << buf << std::endl;
#else
    //print(buf);
    //print("¥n");
#endif
}

static void print_regs(hackcpu_regs_t* p_axi_regs, int& count) {
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

static void execute_commannd(hackcpu_regs_t* p_reg, hackcpu_reg_t word, hackcpu_reg_t length, const hackcpu_reg_t* params, int& reg_count) {
    axireg_start_command(p_reg, word, length, params);
    do {
#ifdef VITIS_HLS_SIM
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

static void config_reset(hackcpu_regs_t* p_reg, hackcpu_reg_t config, int& reg_count) {
    hackcpu_reg_t params[1];
    params[0] = config;
    execute_commannd(p_reg, SET_RESET_CONFIG, 1, params, reg_count);
}

static void load_rom(hackcpu_regs_t* p_reg, const hackcpu_reg_t* rom, int rom_length, int& reg_count) {
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

static void normal_operation(hackcpu_regs_t* p_reg, int& reg_count) {
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
#endif
