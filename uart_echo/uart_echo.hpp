#include <ap_int.h>

// UART Lite レジスタのオフセット
#define RX_FIFO_OFFSET  0x0
#define TX_FIFO_OFFSET  0x1
#define STAT_REG_OFFSET  0x2
#define CTRL_REG_OFFSET  0x3


void uart_echo(
	bool start,
	ap_uint<1> interrupt,
    volatile unsigned int *uart_reg_,
	volatile char& debug_phase_,
	volatile char& debug_rx_data_,
	char debug_injection
);
