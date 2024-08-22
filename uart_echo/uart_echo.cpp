#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include "uart_echo.hpp"

#define UART_COMM_EOF (0x1a)

static bool initialized = false;
static int phase = -1;
static char rx_data = 0;

void uart_echo(
	bool start,
	ap_uint<1> interrupt,
    volatile unsigned int *uart_reg,
	volatile char& debug_phase_,
	volatile char& debug_rx_data_,
	char debug_injection
) {
	#pragma HLS INTERFACE ap_none port=start
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct bundle=AXIM depth=20 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE ap_none port=debug_phase_
	#pragma HLS INTERFACE ap_none port=debug_rx_data_
    #pragma HLS INTERFACE ap_none port=return


	// ボーレート設定（例：115200 bps）
	// 注: 実際のボーレート設定はUART Lite IPの設定に依存します
	// ここでは例として設定が必要な場合を想定しています


	debug_phase_ = phase = 0;
	debug_rx_data_ = rx_data = 0;
	if (start && !initialized) {
		debug_phase_ = phase = 2;
		initialized = true;
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // ソフトウェアリセット
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // リセット解除
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX割り込みを有効化

	} else if (start && debug_injection) {

		// TXFIFOが満杯でないか確認
		while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
		// データをTXFIFOに書き込む
		uart_reg[TX_FIFO_OFFSET] = rx_data+1;
		debug_phase_ = phase = 10;

	} else if (start) {

		debug_phase_ = phase = 6;
		// RXFIFOからデータを読み取る
		while((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 0) {}
		rx_data = uart_reg[RX_FIFO_OFFSET];
		debug_rx_data_ = rx_data;
		//if (rx_data == UART_COMM_EOF) return;
		debug_phase_ = phase = 5;
		// TXFIFOが満杯でないか確認
		while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
		// データをTXFIFOに書き込む
		uart_reg[TX_FIFO_OFFSET] = rx_data+1;
		debug_phase_ = phase = 8;
    }
}

