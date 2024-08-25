#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <cstring>
#include "uart_echo.hpp"

#define UART_COMM_EOF (0x1a)

static bool initialized = false;
static int phase = -1;
static char rx_data = 0;
static char buf[32] = {0};
static int bufp = 0;

static void send_char(volatile unsigned int *uart_reg, const char c) {
	// TXFIFO�����t�łȂ����m�F
	while ((uart_reg[STAT_REG_OFFSET] & 0x00000008)) {};
	// �f�[�^��TXFIFO�ɏ�������
	uart_reg[TX_FIFO_OFFSET] = c;
}

static void send_str(volatile unsigned int *uart_reg, const char *s, int length=sizeof(buf)) {
	int pc = 0;
	while ((s[pc] != 0) && (pc < length)) {
		send_char(uart_reg, s[pc]);
		pc++;
	}
}

// �����񂩂� int �ɕϊ�
static void stoi(const char *b, int digits) {
	int ret = 0;
	for (int i = 0; i < digits; i++) {
		// ToDo
	}
}

static void execute(volatile unsigned int *uart_reg) {
	char *pBuf = buf;

	switch (pBuf[0]) {
	case 't':
		send_str(uart_reg, "Command: test ");
		send_str(uart_reg, &pBuf[2], 4);
		send_str(uart_reg, " ");
		send_str(uart_reg, &pBuf[7], 8);
		send_str(uart_reg, "\n");
		break;
	default:
		send_str(uart_reg, "Unknown command: ");
		send_str(uart_reg, pBuf, bufp);
		send_str(uart_reg, "\n");
		break;
	}
}

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

		send_str(uart_reg, "> ");

	} else if (start && debug_injection) {

		// �f�[�^��TXFIFO�ɏ�������
		send_char(uart_reg, debug_injection);
		debug_phase_ = phase = 10;
		// RXFIFO����f�[�^��ǂݎ��
		while((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 0) {}
		debug_rx_data_ = rx_data = uart_reg[RX_FIFO_OFFSET];

	} else if (start) {

		debug_phase_ = phase = 6;
		// RXFIFO����f�[�^��ǂݎ��
		while((uart_reg[STAT_REG_OFFSET] & 0x00000001) == 0) {}
		rx_data = uart_reg[RX_FIFO_OFFSET];
		debug_rx_data_ = rx_data;
		//if (rx_data == UART_COMM_EOF) return;
		debug_phase_ = phase = 5;

		debug_phase_ = phase = 6;

		if (rx_data == '\r') {
			// �R�}���h���s
			execute(uart_reg);
			send_str(uart_reg, "\n\r> ");
			memset(buf, 0, sizeof(buf));
			bufp = 0;
			debug_phase_ = phase = 7;
		} else if (rx_data == '\b') {
			// echo
			send_char(uart_reg, rx_data);
			// Backspace
			if (bufp > 0) {
				bufp--;
			}
			debug_phase_ = phase = 8;
		} else {
			// echo
			send_char(uart_reg, rx_data);
			// �ꕶ���ς�
			if (bufp < sizeof(buf)) {
				buf[bufp++] = rx_data;
			}
			debug_phase_ = phase = 9;
		}
    }
}

