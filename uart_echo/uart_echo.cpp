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
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct bundle=AXIM depth=20 // depth�𐳂����ݒ肵�Ȃ���Co-sim�����܂������Ȃ�
	#pragma HLS INTERFACE ap_none port=debug_phase_
	#pragma HLS INTERFACE ap_none port=debug_rx_data_
    #pragma HLS INTERFACE ap_none port=return


	// �{�[���[�g�ݒ�i��F115200 bps�j
	// ��: ���ۂ̃{�[���[�g�ݒ��UART Lite IP�̐ݒ�Ɉˑ����܂�
	// �����ł͗�Ƃ��Đݒ肪�K�v�ȏꍇ��z�肵�Ă��܂�


	debug_phase_ = phase = 0;
	debug_rx_data_ = rx_data = 0;
	if (start && !initialized) {
		debug_phase_ = phase = 2;
		initialized = true;
		uart_reg[CTRL_REG_OFFSET] = 0x00000003;  // �\�t�g�E�F�A���Z�b�g
		uart_reg[CTRL_REG_OFFSET] = 0x00000000;  // ���Z�b�g����
		uart_reg[CTRL_REG_OFFSET] = 0x00000010;  // RX���荞�݂�L����

	} else if (start && debug_injection) {

		// TXFIFO�����t�łȂ����m�F
		while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
		// �f�[�^��TXFIFO�ɏ�������
		uart_reg[TX_FIFO_OFFSET] = rx_data+1;
		debug_phase_ = phase = 10;

	} else if (start) {

		debug_phase_ = phase = 6;
		// RXFIFO����f�[�^��ǂݎ��
		while((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 0) {}
		rx_data = uart_reg[RX_FIFO_OFFSET];
		debug_rx_data_ = rx_data;
		//if (rx_data == UART_COMM_EOF) return;
		debug_phase_ = phase = 5;
		// TXFIFO�����t�łȂ����m�F
		while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
		// �f�[�^��TXFIFO�ɏ�������
		uart_reg[TX_FIFO_OFFSET] = rx_data+1;
		debug_phase_ = phase = 8;
    }
}

