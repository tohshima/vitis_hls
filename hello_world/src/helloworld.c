/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "sleep.h"
#include "../../vidoutgen/vidoutgen_def.hpp"
#include "../../hackcpu/axireg_if_def.hpp"

int main()
{
    init_platform();

    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x4);  // reset
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x8);  // gen-lock
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x5C, 0x14000000);  // start addr
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x54, 1280*4);  // h size
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x58, 1280*4 /*0x01001000*/);  // stride
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x83);  // enable
    XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x50, 720);  // v size start dma

#if 0
    for (char j = 0; ; j++) {
        unsigned int *p = (unsigned int*)0x14000000;
        for (int i = 0; i < 1280*720; i++) {
            *p++ = 0x00+((j*3 & 0xFF)<< 16)+((j*2 & 0xFF)<< 8)+((j*1 & 0xFF)<< 0); // ABGR       
        }
        Xil_DCacheFlush();
        usleep(100*1000);
    }
#else
    unsigned int *p = (unsigned int*)0x14000000;
    for (int i = 0; i < 1280*720; i++) {
        *p++ = rand(); // ABGR       
    }
    Xil_DCacheFlush();
    usleep(100*1000);
#endif


    while (1) {
        vidoutgen_quick_test(XPAR_VIDOUTGEN_0_BASEADDR+0x10);
        test_bench_axireg(XPAR_HACKCPU_IF_0_BASEADDR+0x80);
        usleep(2000*1000);
    }
    cleanup_platform();
    return 0;
}
