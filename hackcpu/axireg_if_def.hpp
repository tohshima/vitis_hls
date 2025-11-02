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

static inline void axireg_set_uart_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control |= (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_clear_uart_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control &= ~(1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline bool axireg_is_uart_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->uart_control & (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_set_uart_disp_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control |= (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline void axireg_clear_uart_disp_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->uart_control &= ~(1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_is_uart_disp_enable(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->uart_control & (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_start_command(volatile hackcpu_regs_t* p_reg, hackcpu_reg_t word, hackcpu_reg_t length, const hackcpu_reg_t params[]) {
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
static inline void axireg_set_command_start(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_control |= (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_clear_command_start(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_control &= ~(1 << CMD_CTRL_START_BIT);
}
static inline bool axireg_is_command_start(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_control & (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_set_command_busy(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status |= (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_clear_command_busy(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status &= ~(1 << CMD_STAT_BUSY_BIT);
}
static inline bool axireg_is_command_busy(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_status & (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_set_command_done(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status |= (1 << CMD_STAT_DONE_BIT);
}
static inline void axireg_clear_command_done(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    p_reg->command_status &= ~(1 << CMD_STAT_DONE_BIT);
}
static inline bool axireg_is_command_done(volatile hackcpu_regs_t* p_reg) {
    #pragma HLS INLINE
    return p_reg->command_status & (1 << CMD_STAT_DONE_BIT);
}
static inline hackcpu_reg_t axireg_get_command_result(volatile hackcpu_regs_t* p_reg, hackcpu_reg_t params[]) {
    #pragma HLS INLINE
    hackcpu_reg_t length = p_reg->command_out_num_params;
    for (hackcpu_reg_t i = 0; i < length; i++) {
        params[i] = p_reg->command_out_params[i];
    }
    return p_reg->command_out_status;
}
#endif //  __AXIREG_IF_DEF_HPP__