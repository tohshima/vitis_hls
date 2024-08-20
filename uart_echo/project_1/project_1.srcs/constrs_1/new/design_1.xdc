

create_clock -period 20.000 -name VIRTUAL_clk_out1_design_1_clk_wiz_0 -waveform {0.000 10.000}
set_input_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -min -add_delay 4.000 [get_ports reset]
set_input_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -max -add_delay 4.000 [get_ports reset]
set_input_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -min -add_delay 4.000 [get_ports usb_uart_rxd]
set_input_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -max -add_delay 4.000 [get_ports usb_uart_rxd]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -min -add_delay 0.000 [get_ports usb_uart_txd]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_0] -max -add_delay 4.000 [get_ports usb_uart_txd]
set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets clk]
