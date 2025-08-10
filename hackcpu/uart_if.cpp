// UART IF module for hackcpu
#include "hackcpu.hpp"
#include "uart_if.hpp"
#ifndef __SYNTHESIS__
#include "../common/uart_comm.hpp"
#endif

#if !defined(__SYNTHESIS__)
//#define USE_COM "COM2"
//uart_comm uart_comm("\\\\.\\" USE_COM);  // ポートを開く
#define USE_COM "/tmp/ttyV0" // get a virrtual com port by: socat pty,raw,echo=0,link=/tmp/ttyV0 pty,raw,echo=0,link=/tmp/ttyV1
uart_comm uart_comm(USE_COM);  // ポートを開く
#endif

#if defined(__SYNTHESIS__)
static bool is_not_tx_fifo_full(volatile unsigned int *uart_reg) {
#ifdef USE_ZYNQ_PS_UART
    return (uart_reg[XUARTPS_SR_OFFSET] & 0x00000010) == 0;
#else
    // UART lite IP
    return (uart_reg[STAT_REG_OFFSET] & 0x00000008) == 0;
#endif
}
static void write_to_tx_fifo(volatile unsigned int *uart_reg, char c) {
#ifdef USE_ZYNQ_PS_UART
    uart_reg[XUARTPS_FIFO_OFFSET] = c;
#else
    // UART lite IP
    uart_reg[TX_FIFO_OFFSET] = c;
#endif
}
#endif

static void send_chars(volatile unsigned int *uart_reg, hls::stream<char>& uart_out) {
     // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=16
	#pragma HLS INTERFACE axis port=uart_out depth=1

#if !defined(__SYNTHESIS__)
	char uo[2048];
	size_t length = 0;
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
        if (is_not_tx_fifo_full(uart_reg)) {
            // データをTXFIFOに書き込む
            write_to_tx_fifo(uart_reg, uart_out.read());
        } else {
            // 満杯だったらいったん中断して次の回に
            break;
        }
	}
#endif
}


#if defined(__SYNTHESIS__)
static bool is_not_rx_fifo_empty(volatile unsigned int *uart_reg) {
#ifdef USE_ZYNQ_PS_UART
    return (uart_reg[XUARTPS_SR_OFFSET] & 0x00000002) == 0;
#else
    // UART lite IP
    return (uart_reg[STAT_REG_OFFSET] & 0x00000001) == 1;
#endif
}
static char read_from_rx_fifo(volatile unsigned int *uart_reg) {
#ifdef USE_ZYNQ_PS_UART
    return uart_reg[XUARTPS_FIFO_OFFSET];
#else
    // UART lite IP
    return uart_reg[RX_FIFO_OFFSET];
#endif
}
#endif

static bool get_token(
    volatile unsigned int *uart_reg,
    hls::stream<ap_uint<8*TOKEN_SIZE>>& uart_in,
    volatile bool& sim_exit
) {
    // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=16 
	#pragma HLS INTERFACE axis port=uart_in depth=4

    static char debug_rx_data_ = 0;
	static int char_index = 0;
	static ap_uint<8*TOKEN_SIZE> token = 0;
	static bool key_in_flag = false;

#if !defined(__SYNTHESIS__) 
    char read_buf[1];
    size_t bytes_read = 0;
	while ((uart_comm.read_data(read_buf, sizeof(read_buf), bytes_read)) &&
						(bytes_read == sizeof(read_buf))) {
		debug_rx_data_ = read_buf[0];
#else
	if (is_not_rx_fifo_empty(uart_reg)) {
		debug_rx_data_ = read_from_rx_fifo(uart_reg);
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
    #ifdef USE_ZYNQ_PS_UART
    bool start,
    #endif
    bool& sim_exit
) {
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=16 // depthを正しく設定しないとCo-simがうまくいかない
	#pragma HLS INTERFACE axis port=uart_in depth=32
	#pragma HLS INTERFACE axis port=uart_out depth=4
    #ifdef USE_ZYNQ_PS_UART
    #pragma HLS INTERFACE s_axillite register port=start
    #endif
    #pragma HLS INTERFACE ap_ctrl_none port=return

    static bool initialized = false;

	// ボーレート設定（例：115200 bps）
	// 注: 実際のボーレート設定はUART Lite IPの設定に依存します

    sim_exit = false;
	if (!initialized) {
		initialized = true;
        #ifndef USE_ZYNQ_PS_UART
        // for UART lite IP
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // ソフトウェアリセット
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // リセット解除
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX割り込みを有効化
        #endif
	} else 
    #ifdef USE_ZYNQ_PS_UART
    if (start)
    #endif
    {
		//#pragma HLS DATAFLOW
		//while (1) {
			get_token(uart_reg, uart_in, sim_exit);
			send_chars(uart_reg, uart_out);
		//}
    }
}
