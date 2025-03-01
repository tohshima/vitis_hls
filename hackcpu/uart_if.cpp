// UART IF module for hackcpu
#include "hackcpu.hpp"
#include "uart_if.hpp"
#ifndef __SYNTHESIS__
#include "../common/uart_comm.hpp"
#endif

#ifndef __SYNTHESIS__
#define USE_COM "COM2"
uart_comm uart_comm("\\\\.\\" USE_COM);  // COM2ポートを開く
#endif

static void send_chars(volatile unsigned int *uart_reg, hls::stream<char>& uart_out) {
     // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20
	#pragma HLS INTERFACE axis port=uart_out depth=128

#ifndef __SYNTHESIS__
	char uo[128];
	DWORD length = 0;
	for (int i = 0; i < sizeof(uo); i++) {
		if (uart_out.empty()) break;
		uo[i] = uart_out.read();
		length++;
	}
	if (length) {
		uart_comm.write_data(uo, length);
	}
#else
	while (!uart_out.empty()) {
    	// TXFIFOが満杯でないか確認
        if ((uart_reg[STAT_REG_OFFSET] & 0x00000008) == 0) {
            // データをTXFIFOに書き込む
            uart_reg[TX_FIFO_OFFSET] = uart_out.read();

        } else {
            // 満杯だったらいったん中断して次の回に
            break;
        }
	}
#endif
}


static bool get_token(
    volatile unsigned int *uart_reg,
    hls::stream<ap_uint<8*TOKEN_SIZE>>& uart_in,
    volatile bool& sim_exit
) {
    // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 
	#pragma HLS INTERFACE axis port=uart_in depth=32

    static char debug_rx_data_ = 0;
	static int char_index = 0;
	static ap_uint<8*TOKEN_SIZE> token = 0;
	static bool key_in_flag = false;

#ifndef __SYNTHESIS__
    char read_buf[1];
    DWORD bytes_read = 0;
	while ((uart_comm.read_data(read_buf, sizeof(read_buf), bytes_read)) &&
						(bytes_read == sizeof(read_buf))) {
		debug_rx_data_ = read_buf[0];
#else
	if ((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 1) {
		debug_rx_data_ = uart_reg[RX_FIFO_OFFSET];
#endif
        if (debug_rx_data_ == 'X') {
            sim_exit = true;
            return true;
        }
		token |= (debug_rx_data_ << 8*char_index);
		char_index++;
		if (char_index == TOKEN_SIZE) {
			uart_in.write(token);
			char_index = 0;
			token = 0;
			return true;
		}
	}
	return false;
}

void uart_if(
	unsigned int *uart_reg,
	hls::stream<token_word_t>& uart_in,
	hls::stream<char>& uart_out,
    bool& sim_exit
) {
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE axis port=uart_in depth=32
	#pragma HLS INTERFACE axis port=uart_out depth=128
    #pragma HLS INTERFACE ap_none port=return

    static bool initialized = false;

	// ボーレート設定（例：115200 bps）
	// 注: 実際のボーレート設定はUART Lite IPの設定に依存します

    sim_exit = false;
	if (!initialized) {
		initialized = true;
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // ソフトウェアリセット
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // リセット解除
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX割り込みを有効化

	} else {
		//#pragma HLS DATAFLOW
		//while (1) {
			get_token(uart_reg, uart_in, sim_exit);
			send_chars(uart_reg, uart_out);
		//}
    }
}
