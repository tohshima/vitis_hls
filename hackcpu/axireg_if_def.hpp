// Hack CPU AXI register IF definition
#ifndef __AXIREG_IF_DEF_HPP__
#define __AXIREG_IF_DEF_HPP__

#ifndef VITIS_HLS_SIM 
// for test bench  &  PS program
#include <cstdint>
typedef uint16_t axi_reg_t;

#else
// vitis hls environment
#define AXI_REG_WIDTH 16
typedef ap_uint<AXI_REG_WIDTH> axi_reg_t;

#endif

typedef struct {
    axi_reg_t uart_control;
    axi_reg_t command_control;
    axi_reg_t command_status;
    axi_reg_t command_in_word;
    axi_reg_t command_in_num_params;
    axi_reg_t command_in_params[18];
    axi_reg_t command_out_status;
    axi_reg_t command_out_num_params;
    axi_reg_t command_out_params[16];
} axi_regs_t __attribute__((__packed__)) __attribute__((aligned(2)));

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

static inline void axireg_set_uart_enable(axi_regs_t* p_reg) {
    p_reg->uart_control |= (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_clear_uart_enable(axi_regs_t* p_reg) {
    p_reg->uart_control &= ~(1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline bool axireg_is_uart_enable(axi_regs_t* p_reg) {
    return p_reg->uart_control & (1 << UART_CONTROL_IF_ENABLE_BIT);
}
static inline void axireg_set_uart_disp_enable(axi_regs_t* p_reg) {
    p_reg->uart_control |= (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline void axireg_clear_uart_disp_enable(axi_regs_t* p_reg) {
    p_reg->uart_control &= ~(1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_is_uart_disp_enable(axi_regs_t* p_reg) {
    return p_reg->uart_control & (1 << UART_CONTROL_DISP_OUT_BIT);
}
static inline bool axireg_start_command(axi_regs_t* p_reg, axi_reg_t word, axi_reg_t length, const axi_reg_t params[]) {
    if (p_reg->command_status & (1 << CMD_STAT_BUSY_BIT)) return false;
    p_reg->command_status = 0;
    p_reg->command_in_word = word;
    p_reg->command_in_num_params = length;
    for (axi_reg_t i = 0; i < length; i++) {
        p_reg->command_in_params[i] = params[i];
    }
    p_reg->command_control |= (1 << CMD_CTRL_START_BIT);
    return true;
}
static inline void axireg_set_command_start(axi_regs_t* p_reg) {
    p_reg->command_control |= (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_clear_command_start(axi_regs_t* p_reg) {
    p_reg->command_control &= ~(1 << CMD_CTRL_START_BIT);
}
static inline bool axireg_is_command_start(axi_regs_t* p_reg) {
    return p_reg->command_control & (1 << CMD_CTRL_START_BIT);
}
static inline void axireg_set_command_busy(axi_regs_t* p_reg) {
    p_reg->command_status |= (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_clear_command_busy(axi_regs_t* p_reg) {
    p_reg->command_status &= ~(1 << CMD_STAT_BUSY_BIT);
}
static inline bool axireg_is_command_busy(axi_regs_t* p_reg) {
    return p_reg->command_status & (1 << CMD_STAT_BUSY_BIT);
}
static inline void axireg_set_command_done(axi_regs_t* p_reg) {
    p_reg->command_status |= (1 << CMD_STAT_DONE_BIT);
}
static inline void axireg_clear_command_done(axi_regs_t* p_reg) {
    p_reg->command_status &= ~(1 << CMD_STAT_DONE_BIT);
}
static inline bool axireg_is_command_done(axi_regs_t* p_reg) {
    return p_reg->command_status & (1 << CMD_STAT_DONE_BIT);
}
static inline axi_reg_t axireg_get_command_result(axi_regs_t* p_reg, axi_reg_t params[]) {
    axi_reg_t length = p_reg->command_out_num_params;
    for (axi_reg_t i = 0; i < length; i++) {
        params[i] = p_reg->command_out_params[i];
    }
    return p_reg->command_out_status;
}
#endif //  __AXIREG_IF_DEF_HPP__