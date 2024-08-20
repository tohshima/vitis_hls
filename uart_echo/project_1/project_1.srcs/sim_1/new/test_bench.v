`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/08/15 19:51:16
// Design Name: 
// Module Name: test_bench
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module test_bench();


  // �p�����[�^
  parameter CLK_PERIOD = 20; // 50MHz �N���b�N
  parameter BAUD_RATE = 9600;
  parameter BIT_PERIOD = 1000000000 / BAUD_RATE; // �i�m�b�P��

  // �e�X�g�x���`�̐M��
  reg clk;
  reg rst_n;
  reg rx;
  wire tx;

  // AXI�C���^�[�t�F�[�X�M���i�K�v�ɉ����Ēǉ��j
  // ...

  // UART�g�����X�~�b�^�̃^�X�N
  reg [7:0] tx_data;
  reg tx_start;
  integer i;

  task uart_send_byte;
    input [7:0] data;
    begin
      tx_data = data;
      tx_start = 1'b1;
      #(BIT_PERIOD);
      tx_start = 1'b0;
      
      // �X�^�[�g�r�b�g
      rx = 1'b0;
      #(BIT_PERIOD);
      
      // �f�[�^�r�b�g
      for (i = 0; i < 8; i = i + 1) begin
        rx = tx_data[i];
        #(BIT_PERIOD);
      end
      
      // �X�g�b�v�r�b�g
      rx = 1'b1;
      #(BIT_PERIOD);
    end
  endtask

  // �N���b�N����
  initial begin
    clk = 0;
    forever #(CLK_PERIOD/2) clk = ~clk;
  end

  // �e�X�g�V�[�P���X
  initial begin
    // ������
    rst_n = 0;
    rx = 1'b1; // �A�C�h�����
    #(CLK_PERIOD * 10);
    rst_n = 1;
    
    // �e�X�g�f�[�^�̑��M
    #(CLK_PERIOD * 1000);
    uart_send_byte("H");
    #(BIT_PERIOD * 4);
    uart_send_byte("e");
    #(BIT_PERIOD * 4);
    uart_send_byte("l");
    #(BIT_PERIOD * 4);
    uart_send_byte("l");
    #(BIT_PERIOD * 4);
    uart_send_byte("o");
    
    // �e�X�g�I��
    #(CLK_PERIOD * 1000);
    $finish;
  end

  // UART IP�̃C���X�^���X��
  design_1_wrapper design_1_wrapper (
    .reset(rst_n),
    .sys_clock(clk),
    .usb_uart_rxd(rx),
    .usb_uart_txd(tx)
  );

endmodule
