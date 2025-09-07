// HACKCPU UART IF top module
#include <hls_task.h>

#include "hackcpu.hpp"
#include "start_tasks.hpp"
#include "uart_if.hpp"
#include "hackcpu_uart.hpp"

#ifdef USE_PYNQ_BUTTON
static void sync_led_active(
    volatile ap_uint<1> btn_smp_clk,
    volatile ap_uint<1>& led_active_out,
    hls::stream< ap_uint<1> >& led_active
) {
#pragma HLS inline
    static ap_uint<1> prev_clk = 0;
    ap_uint<1> curr_clk = btn_smp_clk;
    static ap_uint<1> synched_active = 0;
    static ap_uint<1> curr_active = 0;
    
    if (!led_active.empty()) {
        curr_active = led_active.read();
    }
    synched_active |= curr_active;

    if (prev_clk != curr_clk) {
        prev_clk = curr_clk;
        if (curr_clk == 0) {
            // falling edge of curr_clk
            led_active_out = synched_active;
            synched_active = 0;
        }
    }
}

static void check_buttons(
    volatile ap_uint<1> button_in0,
    volatile ap_uint<1> button_in1,
    volatile ap_uint<1> button_in2,
    volatile ap_uint<1> button_in3,
    volatile ap_uint<1> btn_smp_clk,
    volatile ap_uint<1>& led_btn_L_out,
    volatile ap_uint<1>& led_btn_R_out,
	hls::stream<token_word_t>& uart_in,
    volatile ap_uint<8>& debug_phase
) {
#pragma HLS inline
    static ap_uint<1> prev_clk = 0;
    static ap_uint<1> prev_btn0 = 0;
    static ap_uint<1> prev_btn1 = 0;
    static ap_uint<1> prev_btn2 = 0;
    ap_uint<1> curr_clk = btn_smp_clk;
    ap_uint<1> curr_btn0 = button_in0;
    ap_uint<1> curr_btn1 = button_in1;
    ap_uint<1> curr_btn2 = button_in2;
    debug_phase = 0x31;

    if (curr_clk != prev_clk) {
        prev_clk = btn_smp_clk;
        // C-L button
        if (curr_btn2 != prev_btn2) {
            debug_phase = 0x32;
            prev_btn2 = curr_btn2;
            if (curr_btn2) {
                debug_phase = 0x33;
                uart_in.write(PYNQ_BUTTON_CODE_CLEFT);
            } else {
                uart_in.write(PYNQ_BUTTON_CODE_NONE);
            }        
        }
        // C-R button
        if (curr_btn1 != prev_btn1) {
            debug_phase = 0x34;
            prev_btn1 = curr_btn1;
            if (curr_btn1) {
                debug_phase = 0x35;
                uart_in.write(PYNQ_BUTTON_CODE_CRIGHT);
            } else {
                uart_in.write(PYNQ_BUTTON_CODE_NONE);
            }        
        }
        // R button
        if (curr_btn0 != prev_btn0) {
            debug_phase = 0x36;
            prev_btn0 = curr_btn0;
            if (curr_btn0) {
                debug_phase = 0x37;
                uart_in.write(PYNQ_BUTTON_CODE_RIGHT);
            }      
        }
    }
    led_btn_L_out = curr_btn2;
    led_btn_R_out = curr_btn1;
}
#endif

int hackcpu_uart(
    #ifdef USE_ZYNQ_PS_UART
    ap_uint<1> uart_start,
    #endif
    #ifdef USE_PYNQ_BUTTON
    volatile ap_uint<1> button_in0,
    volatile ap_uint<1> button_in1,
    volatile ap_uint<1> button_in2,
    volatile ap_uint<1> button_in3,
    volatile ap_uint<1> btn_smp_clk,
    volatile ap_uint<1>& led_btn_L_out,
    volatile ap_uint<1>& led_btn_R_out,
    volatile ap_uint<1>& led_active_out,
    #endif
	volatile unsigned int *uart_reg,
    volatile ap_uint<8>& debug_phase
) {
    #ifdef USE_ZYNQ_PS_UART
    #pragma HLS INTERFACE s_axilite register port=uart_start
    #endif
    #ifdef USE_PYNQ_BUTTON
    #pragma HLS INTERFACE ap_none port=button_in0    
    #pragma HLS INTERFACE ap_none port=button_in1    
    #pragma HLS INTERFACE ap_none port=button_in2    
    #pragma HLS INTERFACE ap_none port=button_in3    
    #pragma HLS INTERFACE ap_none port=btn_smp_clk    
    #pragma HLS INTERFACE ap_none port=led_btn_L_out    
    #pragma HLS INTERFACE ap_none port=led_btn_R_out    
    #pragma HLS INTERFACE ap_none port=led_active_out    
    #endif
    #pragma HLS INTERFACE m_axi port=uart_reg offset=direct depth=20 // depthを正しく設定しないとCo-simがうまくいかない
    //#ifdef USE_ZYNQ_PS_UART
    //#pragma HLS INTERFACE s_axilite port=return
    //#else
    #pragma HLS INTERFACE ap_ctrl_none port=return
    //#endif

    #pragma HLS DATAFLOW
    #ifdef USE_PYNQ_BUTTON
	hls_thread_local hls::stream< ap_uint<1> > led_active;
    #pragma HLS STREAM variable=led_active_out depth=1
    #endif
	hls_thread_local hls::stream<token_word_t> uart_in;
    #pragma HLS STREAM variable=uart_in depth=32
	hls_thread_local hls::stream<char> uart_out;
    #pragma HLS STREAM variable=uart_out depth=128
	
    start_tasks(
        #ifdef USE_PYNQ_BUTTON
        led_active,
        #endif
        uart_in, uart_out);

#ifndef SIM_TATSKS
    bool sim_exit = false;
    #ifndef USE_ZYNQ_PS_UART
	for(;;) 
    #endif
    {
        #pragma HLS PIPELINE
        debug_phase = 0x10;
        #ifdef USE_ZYNQ_PS_UART
        if (uart_start) 
        #endif
        {
            uart_if(uart_reg, uart_in, uart_out, sim_exit, debug_phase);
        }

        #ifdef USE_PYNQ_BUTTON
        debug_phase = 0x11;
        sync_led_active(btn_smp_clk, led_active_out, led_active);
        
        debug_phase = 0x13;
        check_buttons(button_in0, button_in1, button_in2, button_in3, 
            btn_smp_clk, led_btn_L_out, led_btn_R_out, uart_in, debug_phase);
        #endif
    #ifndef USE_ZYNQ_PS_UART
        if (sim_exit) return 0;
    #endif
	}
    return 0;
#endif
}
