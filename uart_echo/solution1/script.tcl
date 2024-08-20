############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
############################################################
open_project uart_echo
set_top uart_echo
add_files uart_echo/uart_echo.hpp
add_files uart_echo/uart_echo.cpp
add_files -tb uart_echo/test_bench.cpp -cflags "-Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7a35t-csg324-1}
create_clock -period 20 -name default
config_export -format ip_catalog -output C:/Users/tetsu/vitis_hls/uart_echo -rtl verilog
source "./uart_echo/solution1/directives.tcl"
csim_design -clean
csynth_design
cosim_design -wave_debug -trace_level all
export_design -rtl verilog -format ip_catalog -output C:/Users/tetsu/vitis_hls/uart_echo
