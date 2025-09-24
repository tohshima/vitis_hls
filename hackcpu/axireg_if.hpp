// AXI rregister IF module for hackcpu
#pragma once
#include <ap_int.h>
#include <hls_stream.h>

#define AXIREG_DATA_WIDTH (32)
#define AXIREG_ADDR_WIDTH (8)

typedef ap_uint<AXIREG_DATA_WIDTH> axireg_data_t;
typedef ap_uint<AXIREG_ADDR_WIDTH>  axireg_addr_t;
typedef struct {
    axireg_data_t data;
    axireg_addr_t addr;
} __attribute__((packed)) axireg_ext_t;

typedef struct {
    bool uart_enable;
    bool uard_disp_enable;
} regs_t;

typedef enum {
    AXIREG_IF_UART_ENABLE_ADDR  = 0x00,
    AXIREG_IF_COMMAND_IN_ADDR   = 0x04,
    AXIREG_IF_COMMAND_OUT_ADDR  = 0x08
} axireg_if_addr_e;

void axireg_task(
    hls::stream<axireg_ext_t>& reg_ext_in,
    hls::stream<axireg_ext_t>& reg_ext_out,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<axireg_data_t>& reg_command_in_read,
    hls::stream<axireg_data_t>& reg_command_out_write
);

inline axireg_ext_t make_axireg_val(axireg_addr_t a, axireg_data_t d) {
    axireg_ext_t v;
    v.data = d;
    v.addr = a;
    return v;
}