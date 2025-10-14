// AXI register IF module for hackcpu
#ifndef __AXIREG_IF_HPP
#define __AXIREG_IF_HPP

#include "axireg_if_def.hpp"

void axireg_if(
    axi_regs_t& axi_regs,
    hls::stream<bool>& reg_uart_enable_read,
    hls::stream<bool>& reg_uart_disp_enable_read,
    hls::stream<axi_reg_t>& reg_command_in_read,
    hls::stream<axi_reg_t>& reg_command_out_write
);
#endif