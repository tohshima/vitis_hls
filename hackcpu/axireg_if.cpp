// AXI rregister IF module for hackcpu
#include "hackcpu.hpp"
#include "axireg_if.hpp"

//#define GET_ADDR(v) (((axireg_ext_t)v >> AXIREG_DATA_WIDTH) & ((1 << AXIREG_ADDR_WIDTH)-1))
//#define GET_DATA(v) ((axireg_ext_t)v & (((axireg_ext_t)1 << AXIREG_DATA_WIDTH)-1))


void axireg_if(
    hackcpu_regs_t& axi_regs,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<hackcpu_reg_t>& reg_command_in_read,
    hls::stream<hackcpu_reg_t>& reg_command_out_write
) {
    #pragma HLS INLINE
    //#pragma HLS PIPELINE II=1
    #pragma HLS INTERFACE s_axilite register port=axi_regs   
    #pragma HLS INTERFACE ap_fifo port=reg_uart_enable_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_uart_disp_enable_read depth=1
    #pragma HLS INTERFACE ap_fifo port=reg_command_in_read depth=20
    #pragma HLS INTERFACE ap_fifo port=reg_command_out_write depth=18

    if (!axireg_is_command_busy(&axi_regs)) {
        if (axireg_is_command_start(&axi_regs)) {
            axireg_clear_command_start(&axi_regs);
            axireg_set_command_busy(&axi_regs);
            axireg_clear_command_done(&axi_regs);
            hackcpu_reg_t w = axi_regs.command_in_word;
            hackcpu_reg_t n = axi_regs.command_in_num_params;
            reg_command_in_read.write(w);
            for (hackcpu_reg_t i = 0; i < n; i++) {
                #pragma HLS UNNROLL
                reg_command_in_read.write(axi_regs.command_in_params[i]);
            }            
        }
    } else {
        // WIP
        if (!reg_command_out_write.empty()) {
            hackcpu_reg_t n = reg_command_out_write.read();
            axi_regs.command_out_num_params = n;
            for (hackcpu_reg_t i = 0; i < sizeof(axi_regs.command_out_params)/sizeof(axi_regs.command_out_params[0]); i++) {
                #pragma HLS UNROLL
                if (i < n) {
                    axi_regs.command_out_params[i] = reg_command_out_write.read();
                } else {
                    axi_regs.command_out_params[i] = 0;
                }
            }
            hackcpu_reg_t s = reg_command_out_write.read();
            axi_regs.command_out_status = s;
            axireg_clear_command_busy(&axi_regs);
            axireg_set_command_done(&axi_regs);
        }
    }
    if (!reg_uart_enable_read.full()) {
        reg_uart_enable_read.write(axireg_is_uart_enable(&axi_regs));
    }
    if (!reg_uart_disp_enable_read.full()) {
        reg_uart_disp_enable_read.write(axireg_is_uart_disp_enable(&axi_regs));
    }
}