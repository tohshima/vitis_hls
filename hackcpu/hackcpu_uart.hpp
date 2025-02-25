// HACKCPU UART IF top module 
#ifndef __HACKCPU_UART_HPP__
#define __HACKCPU_UART_HPP__

#include <ap_int.h>
#include <hls_task.h>
#include <hls_stream.h>
#include "hackcpu.hpp"

int hackcpu_uart(
	volatile unsigned int *uart_reg
);
#endif // #ifndef __HACKCPU_UART_HPP__
