#ifndef __VIDOUTGEN_DEF_HPP__
#define __VIDOUTGEN_DEF_HPP__

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
typedef int bool;
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
    VIDOUTGEN_CONTROL_CLS_BIT     = 1,
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
#if 1
#define MAX_WIDTH (1280)
#define MAX_HEIGHT (720)
#else
#define MAX_WIDTH (16) //(1280)
#define MAX_HEIGHT (4) //(720)
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline void vidoutgen_set_control_enable(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->control |= (1 << VIDOUTGEN_CONTROL_ENABLE_BIT);
}
static inline void vidoutgen_clear_control_enable(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->control &= ~(1 << VIDOUTGEN_CONTROL_ENABLE_BIT);
}
static inline bool vidoutgen_get_control_enable(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->control & (1 << VIDOUTGEN_CONTROL_ENABLE_BIT);
}
static inline void vidoutgen_set_control_cls(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->control |= (1 << VIDOUTGEN_CONTROL_CLS_BIT);
}
static inline void vidoutgen_clear_control_cls(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->control &= ~(1 << VIDOUTGEN_CONTROL_CLS_BIT);
}
static inline bool vidoutgen_get_control_cls(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->control & (1 << VIDOUTGEN_CONTROL_CLS_BIT);
}
static inline void vidoutgen_set_bg_width(volatile vidoutgen_regs_t* p_reg, uint32_t width) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->bg_width = width;
}
static inline uint32_t vidoutgen_get_bg_width(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->bg_width;
}
static inline void vidoutgen_set_bg_height(volatile vidoutgen_regs_t* p_reg, uint32_t height) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->bg_height = height;
}
static inline uint32_t vidoutgen_get_bg_height(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->bg_height;
}
static inline void vidoutgen_set_bg_color(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgb_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->bg_col_r = p_color->r;
    p_reg->bg_col_g = p_color->g;
    p_reg->bg_col_b = p_color->b;
}
static inline void vidoutgen_get_bg_color(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgb_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_color->r = p_reg->bg_col_r;
    p_color->g = p_reg->bg_col_g;
    p_color->b = p_reg->bg_col_b;
}
static inline void vidoutgen_get_bg_color_as_rgba(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_color->r = p_reg->bg_col_r;
    p_color->g = p_reg->bg_col_g;
    p_color->b = p_reg->bg_col_b;
    p_color->a = 0;
}
static inline void vidoutgen_set_fg_width(volatile vidoutgen_regs_t* p_reg, uint32_t width) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_width = width;
}
static inline uint32_t vidoutgen_get_fg_width(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->fg_width;
}
static inline void vidoutgen_set_fg_height(volatile vidoutgen_regs_t* p_reg, uint32_t height) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_height = height;
}
static inline uint32_t vidoutgen_get_fg_height(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->fg_height;
}
static inline void vidoutgen_set_fg_offset_x(volatile vidoutgen_regs_t* p_reg, uint32_t x) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_offset_x = x;
}
static inline uint32_t vidoutgen_get_fg_offset_x(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->fg_offset_x;
}
static inline void vidoutgen_set_fg_offset_y(volatile vidoutgen_regs_t* p_reg, uint32_t y) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_offset_y = y;
}
static inline uint32_t vidoutgen_get_fg_offset_y(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->fg_offset_y;
}
static inline void vidoutgen_set_fg_color0(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_col0_r = p_color->r;
    p_reg->fg_col0_g = p_color->g;
    p_reg->fg_col0_b = p_color->b;
    p_reg->fg_col0_a = p_color->a;
}
static inline void vidoutgen_get_fg_color0(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_color->r = p_reg->fg_col0_r;
    p_color->g = p_reg->fg_col0_g;
    p_color->b = p_reg->fg_col0_b;
    p_color->a = p_reg->fg_col0_a;
}
static inline void vidoutgen_set_fg_color1(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_col1_r = p_color->r;
    p_reg->fg_col1_g = p_color->g;
    p_reg->fg_col1_b = p_color->b;
    p_reg->fg_col1_a = p_color->a;
}
static inline void vidoutgen_get_fg_color1(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t* p_color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_color->r = p_reg->fg_col1_r;
    p_color->g = p_reg->fg_col1_g;
    p_color->b = p_reg->fg_col1_b;
    p_color->a = p_reg->fg_col1_a;
}
static inline void vidoutgen_set_buf0_offset_addr(volatile vidoutgen_regs_t* p_reg, uint64_t addr) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->buf0_addr_low = addr & 0xFFFFFFFFul;
    p_reg->buf0_addr_high = (addr >> 32) & 0xFFFFFFFFul;
}
static inline uint64_t vidoutgen_get_buf0_offset_addr(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return ((uint64_t)p_reg->buf0_addr_high << 32) + (uint64_t)p_reg->buf0_addr_low;
}

static void vidoutgen_quick_test(uint32_t reg_address) {
    vidoutgen_regs_t* p_reg = (vidoutgen_regs_t*)reg_address;
    vidoutgen_set_bg_width(p_reg, 1280);
    vidoutgen_set_bg_height(p_reg, 720);
    vidoutgen_rgb_t bg_col = {0x11, 0x11, 0x11};
    vidoutgen_set_bg_color(p_reg, &bg_col);
    vidoutgen_set_fg_width(p_reg, 512);
    vidoutgen_set_fg_height(p_reg, 256);
    vidoutgen_set_fg_offset_x(p_reg, (1280-512)/2);
    vidoutgen_set_fg_offset_y(p_reg, (720-256)/2);
    vidoutgen_rgba_t fg_col0 = {0xFF, 0xFF, 0xFF, 0x00};
    vidoutgen_set_fg_color0(p_reg, &fg_col0);

    vidoutgen_set_buf0_offset_addr(p_reg, 0x14000000ull);

    vidoutgen_set_control_enable(p_reg);
    vidoutgen_set_control_cls(p_reg);    
}

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // #ifndef __VIDOUTGEN_DEF_HPP__
