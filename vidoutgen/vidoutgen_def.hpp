#ifndef __VIDOUTGEN_DEF_HPP__
#define __VIDOUTGEN_DEF_HPP__

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

typedef struct {
    uint32_t control;
    uint32_t bg_width;
    uint32_t bg_height;
    uint8_t  bg_col_r;
    uint8_t  bg_col_g;
    uint8_t  bg_col_b;
    uint8_t  rsv0;
    uint32_t fg_width;
    uint32_t fg_height;
    uint32_t fg_offset_x;
    uint32_t fg_offset_y;
    uint8_t  fg_col0_r;
    uint8_t  fg_col0_g;
    uint8_t  fg_col0_b;
    uint8_t  fg_col0_a;
    uint8_t  fg_col1_r;
    uint8_t  fg_col1_g;
    uint8_t  fg_col1_b;
    uint8_t  fg_col1_a;
    uint32_t buf0_addr_low;
    uint32_t buf0_addr_high;
} vidoutgen_regs_t;

typedef enum{
    VIDOUTGEN_CONTROL_ENABLE_BIT  = 0,
} vidoutgen_control_e;

// RGB format for Video out
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} vidoutgen_rgb_t;

// RGB format on DRAM
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} vidoutgen_rgba_t;

// Hackcpu videointput format
typedef struct {
    uint16_t addr;
    uint16_t data;
} hackcpu_video_t;

#define MAX_BURST_NUM (16)
#define MAX_WIDTH (1280)
#define MAX_HEIGHT (720)

#endif // #ifndef __VIDOUTGEN_DEF_HPP__
