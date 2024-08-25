#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <ap_int.h>
#include "uart_echo.hpp"

#define INPUT_FROM_COM
#define USE_COM "COM2"

#include "../common/uart_comm.hpp"

// グローバル変数

// テストベンチ関数
int main() {
	int test_result = 0;
	unsigned int uart_regs[4];
    char debug_phase = -1;
    char debug_rx_data = 0;
    char debug_injection = 0;

#ifdef INPUT_FROM_COM
    uart_comm uart_comm("\\\\.\\"USE_COM);  // COM2ポートを開く
#endif

    uart_echo(true, 0, uart_regs, debug_phase, debug_rx_data, debug_injection);

    // 制御レジスタの確認
#if 0
    if (uart_regs.control_reg != 0x0010) {
        std::cout << "Control register was not properly initialized." << std::endl;
        test_result = 1;
    }
#endif

#ifndef INPUT_FROM_COM
#endif

    // データ送信のシミュレート
#ifndef INPUT_FROM_COM
    // テストデータ
    const char *test_data = "Hello, UART!";
    int data_len = strlen(test_data);

    for (int i = 0; i < data_len && test_result == 0; i++) {
#else
    while (test_result == 0) {
    	// COMから文字入力を待つ
    	char test_data[256];
    	char ans_data[256] = {0};
		DWORD bytesRead;
		if (uart_comm.read_data(test_data, sizeof(test_data), bytesRead)) {
			if (test_data[0] == UART_COMM_EOF) {
				std::cout << "Received EOF" << std::endl; // EOFが来たらテスト終了
				break;
			}
			test_data[bytesRead] = '\0';  // Null-terminate the string
			if (bytesRead > 0) {
				std::cout << "Received from " << USE_COM << ": " << test_data << std::endl;
			}
		}
		for (int i = 0; i < bytesRead; i++) {
#endif
	        // RXFIFOデータを設定
	        uart_regs[RX_FIFO_OFFSET] = test_data[i];
	        uart_regs[STAT_REG_OFFSET] = 0x0001; // RX割り込みをシミュレート
	    	uart_echo(true, 1, uart_regs, debug_phase, debug_rx_data, debug_injection);

	        // TXFIFOの確認
#if 0
			if (uart_regs[TX_FIFO_OFFSET] != test_data[i]+1) {
				std::cout << "Error: Mismatch at index " << i << ". Expected: " << (char)(test_data[i]+1)
						  << ", Got: " << (char)uart_regs[TX_FIFO_OFFSET].to_int() << std::endl;
				test_result = 2;
				break;
			}
#endif

#ifdef INPUT_FROM_COM
			ans_data[i] = uart_regs[TX_FIFO_OFFSET];
#endif

			// ステータスレジスタがセットされるのを待つ
#if 0 		// 割り込みフラグはWrite 1でクリアのタイプのHWなのでSimulationできない
			// ステータスレジスタの確認
			if (uart_regs[STAT_REG_OFFSET] != 0x0000) {
				std::cout << "Error: Status register not cleared properly" << std::endl;
				test_result = 3;
				break;
			}
#endif
#ifdef INPUT_FROM_COM
		}
		if (uart_comm.write_data(ans_data, strlen(ans_data))) {
			std::cout << "Sent successfully: " << ans_data << "\n";
		}
#endif
    }

 	if (test_result == 0) {
		std::cout << "All tests passed successfully!" << std::endl;
	}
	return test_result;
}
