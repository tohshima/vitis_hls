// helloworld.c

#if 1
//https://adaptivesupport.amd.com/s/article/932553?language=ja// https://adaptivesupport.amd.com/s/article/932553?language=ja
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xv_tpg.h"
#include "xparameters.h"
#include "xhackcpu_if_hw.h"
#include "test_bench_axireg.hpp"

XV_tpg tpg_inst;
int Status;

int main()
{
    init_platform();

    print("Hello World\n\r");

    /* TPG Initialization */
    Status = XV_tpg_Initialize(&tpg_inst, XPAR_XV_TPG_0_BASEADDR);
    if(Status!= XST_SUCCESS)
    {
             xil_printf("TPG configuration failed\r\n");
        return(XST_FAILURE);
    }

    // Set Resolution to 800x600
    XV_tpg_Set_height(&tpg_inst, 720);
    XV_tpg_Set_width(&tpg_inst, 1280);

    // Set Color Space to RGB
    XV_tpg_Set_colorFormat(&tpg_inst, 0x0);

    //Set pattern to color bar
    XV_tpg_Set_bckgndId(&tpg_inst, XTPG_BKGND_RAINBOW_COLOR);

    //Start the TPG
    XV_tpg_EnableAutoRestart(&tpg_inst);
    XV_tpg_Start(&tpg_inst);
    xil_printf("TPG started!\r\n");
    /* End of TPG code*/

    hackcpu_regs_t* p_hackcpu_regs = (hackcpu_regs_t*)(XPAR_HACKCPU_IF_0_BASEADDR + XHACKCPU_IF_CONTROL_ADDR_AXI_REGS_BASE);
    test_bench_axireg(p_hackcpu_regs);

    cleanup_platform();
    return 0;
}
#else
#include "xparameters.h"
#include "xv_tpg.h"
#include "xil_printf.h"
#include "sleep.h"

// 720p パラメータ
#define H_ACTIVE    1280
#define V_ACTIVE    720
#define H_FRONT     110
#define H_SYNC      40
#define H_BACK      220
#define V_FRONT     5
#define V_SYNC      5
#define V_BACK      20

#define H_TOTAL     (H_ACTIVE + H_FRONT + H_SYNC + H_BACK)
#define V_TOTAL     (V_ACTIVE + V_FRONT + V_SYNC + V_BACK)

// カラーフォーマット定数（xvidc.hが無い場合の定義）
#define CSF_RGB    0
#define CSF_YUV444 1
#define CSF_YUV422 2
#define CSF_YUV420 3

// テストパターンID
#define TPG_BKGND_COLOR_BARS    9
#define TPG_BKGND_H_RAMP        1
#define TPG_BKGND_V_RAMP        2
#define TPG_BKGND_SOLID_RED     4
#define TPG_BKGND_CHECKER_BOARD 15

// グローバル変数
XV_tpg tpg;
XV_tpg_Config *tpg_config;

/**
 * TPG初期化
 */
int init_tpg(u16 DeviceId)
{
    int Status;
    
    tpg_config = XV_tpg_LookupConfig(DeviceId);
    if (!tpg_config) {
        xil_printf("TPG configuration not found\r\n");
        return XST_FAILURE;
    }
    
    Status = XV_tpg_CfgInitialize(&tpg, tpg_config, tpg_config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("TPG initialization failed\r\n");
        return XST_FAILURE;
    }
    
    xil_printf("TPG initialized at 0x%08X\r\n", tpg_config->BaseAddress);
    return XST_SUCCESS;
}

/**
 * TPGを720p用に設定
 */
void setup_tpg_720p(void)
{
    XV_tpg_DisableAutoRestart(&tpg);
    
    // 解像度設定
    XV_tpg_Set_height(&tpg, V_ACTIVE);
    XV_tpg_Set_width(&tpg, H_ACTIVE);
    
    // カラーフォーマット設定（数値で指定）
    XV_tpg_Set_colorFormat(&tpg, CSF_RGB);  // 0 = RGB
    
    // テストパターン選択（カラーバー）
    XV_tpg_Set_bckgndId(&tpg, TPG_BKGND_COLOR_BARS);
    
    // モーション速度設定
    XV_tpg_Set_motionSpeed(&tpg, 4);
    
    // ボックスサイズ設定
    XV_tpg_Set_boxSize(&tpg, 50);
    
    // マスクID設定
    XV_tpg_Set_maskId(&tpg, 0);
    
    xil_printf("TPG configured for 720p60\r\n");
    xil_printf("  Width: %d\r\n", H_ACTIVE);
    xil_printf("  Height: %d\r\n", V_ACTIVE);
}

/**
 * TPG開始
 */
void start_tpg(void)
{
    XV_tpg_EnableAutoRestart(&tpg);
    XV_tpg_Start(&tpg);
    xil_printf("TPG started\r\n");
}

/**
 * TPG停止
 */
void stop_tpg(void)
{
    XV_tpg_DisableAutoRestart(&tpg);
    // XV_tpg_Stop は存在しない場合があるので削除
    xil_printf("TPG stopped\r\n");
}

/**
 * パターン変更
 */
void change_pattern(u32 pattern_id)
{
    XV_tpg_Set_bckgndId(&tpg, pattern_id);
    xil_printf("Pattern changed to %d\r\n", pattern_id);
}

/**
 * メイン関数
 */
int main(void)
{
    int Status;
    
    xil_printf("\r\n");
    xil_printf("=================================\r\n");
    xil_printf("===   TPG 720p Test Start     ===\r\n");
    xil_printf("=================================\r\n");
    
    // TPG DEVICE_IDを確認して使用
    // xparameters.hに定義されている正しいIDを使用
    #ifdef XPAR_XV_TPG_0_DEVICE_ID
        Status = init_tpg(XPAR_XV_TPG_0_DEVICE_ID);
    #elif defined(XPAR_V_TPG_0_DEVICE_ID)
        Status = init_tpg(XPAR_V_TPG_0_DEVICE_ID);
    #else
        Status = init_tpg(0);  // デフォルトで0を使用
        xil_printf("Warning: Using default device ID 0\r\n");
    #endif
    
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: TPG init failed\r\n");
        return XST_FAILURE;
    }
    
    // 720p設定
    setup_tpg_720p();
    
    // TPG開始
    start_tpg();
    
    xil_printf("\r\nDisplaying 720p patterns...\r\n");
    xil_printf("Pattern will change every 3 seconds\r\n\r\n");
    
    // パターン変更ループ
    while(1) {
        xil_printf("Pattern: Color Bars\r\n");
        change_pattern(TPG_BKGND_COLOR_BARS);
        sleep(3);
        
        xil_printf("Pattern: Horizontal Ramp\r\n");
        change_pattern(TPG_BKGND_H_RAMP);
        sleep(3);
        
        xil_printf("Pattern: Vertical Ramp\r\n");
        change_pattern(TPG_BKGND_V_RAMP);
        sleep(3);
        
        xil_printf("Pattern: Solid Red\r\n");
        change_pattern(TPG_BKGND_SOLID_RED);
        sleep(3);
        
        xil_printf("Pattern: Checker Board\r\n");
        change_pattern(TPG_BKGND_CHECKER_BOARD);
        sleep(3);
        
        xil_printf("\r\n--- Restarting pattern loop ---\r\n\r\n");
    }
    
    return XST_SUCCESS;
}
#endif
