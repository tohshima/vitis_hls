// HACKCPU UART IF top module 
#ifndef __HACKCPU_UART_HPP__
#define __HACKCPU_UART_HPP__

#include <ap_int.h>
#include "hackcpu.hpp"

int hackcpu_uart(
	unsigned int *uart_reg
    #ifdef USE_ZYNQ_PS_UART
    ,
    bool& start
    #endif
);
#endif // #ifndef __HACKCPU_UART_HPP__
