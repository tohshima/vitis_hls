// AXI rregister IF module for hackcpu
#include "hackcpu.hpp"
#include "axireg_if.hpp"

//#define GET_ADDR(v) (((axireg_ext_t)v >> AXIREG_DATA_WIDTH) & ((1 << AXIREG_ADDR_WIDTH)-1))
//#define GET_DATA(v) ((axireg_ext_t)v & (((axireg_ext_t)1 << AXIREG_DATA_WIDTH)-1))

static regs_t s_regs = {
    .uart_enable = false,
};

void axireg_task(
    hls::stream<axireg_ext_t>& reg_ext_in,
    hls::stream<axireg_ext_t>& reg_ext_out,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<axireg_data_t>& reg_command_in_read,
    hls::stream<axireg_data_t>& reg_command_out_write
) {
    #pragma HLS INTERFACE axis port=reg_ext_in depth=16   
    #pragma HLS INTERFACE axis port=reg_ext_out depth=16
    #pragma HLS INTERFACE ap_fifo port=reg_uart_enable_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_uart_disp_enable_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_command_in_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_command_out_write depth=1

    if (!reg_ext_in.empty()) {
        axireg_ext_t v = reg_ext_in.read();
        reg_ext_out.write(v);
        axireg_addr_t addr = v.addr;
        axireg_data_t data = v.data;
        
        switch(addr) {
        case AXIREG_IF_UART_ENABLE_ADDR:
            s_regs.uart_enable = data[0];
            s_regs.uard_disp_enable = data[1];
            break;
        case AXIREG_IF_COMMAND_IN_ADDR:
            reg_command_in_read.write(data);
            break;
        default:
            break;
        }
    }
    if (!reg_uart_enable_read.full()) {
        reg_uart_enable_read.write(s_regs.uart_enable);
    }
    if (!reg_uart_disp_enable_read.full()) {
        reg_uart_disp_enable_read.write(s_regs.uard_disp_enable);
    }
    if (!reg_command_out_write.empty() && !reg_ext_out.full()) {
        axireg_data_t d = reg_command_out_write.read();
        reg_ext_out.write(make_axireg_val(AXIREG_IF_COMMAND_OUT_ADDR, d));
    }
}