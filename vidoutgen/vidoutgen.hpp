#ifndef __VIDOUTGEN_HPP__
#define __VIDOUTGEN_HPP__

#include <ap_int.h>
#include <hls_stream.h>

#include "vidoutgen_def.hpp"

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
static inline void vidoutgen_set_bg_color(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgb_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->bg_col_r = color.r;
    p_reg->bg_col_g = color.g;
    p_reg->bg_col_b = color.b;
}
static inline void vidoutgen_get_bg_color(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgb_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    color.r = p_reg->bg_col_r;
    color.g = p_reg->bg_col_g;
    color.b = p_reg->bg_col_b;
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
    p_reg->fg_offset_x = y;
}
static inline uint32_t vidoutgen_get_fg_offset_y(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return p_reg->fg_offset_y;
}
static inline void vidoutgen_set_fg_color0(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_col0_r = color.r;
    p_reg->fg_col0_g = color.g;
    p_reg->fg_col0_b = color.b;
    p_reg->fg_col0_a = color.a;
}
static inline void vidoutgen_get_fg_color0(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    color.r = p_reg->fg_col0_r;
    color.g = p_reg->fg_col0_g;
    color.b = p_reg->fg_col0_b;
    color.a = p_reg->fg_col0_a;
}
static inline void vidoutgen_set_fg_color1(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->fg_col1_r = color.r;
    p_reg->fg_col1_g = color.g;
    p_reg->fg_col1_b = color.b;
    p_reg->fg_col1_a = color.a;
}
static inline void vidoutgen_get_fg_color1(volatile vidoutgen_regs_t* p_reg, vidoutgen_rgba_t& color) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    color.r = p_reg->fg_col1_r;
    color.g = p_reg->fg_col1_g;
    color.b = p_reg->fg_col1_b;
    color.a = p_reg->fg_col1_a;
}
static inline void vidoutgen_set_buf0_offset_addr(volatile vidoutgen_regs_t* p_reg, uint64_t addr) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    p_reg->buf0_addr_low = addr & 0xFFFFFFFFul;
    p_reg->buf0_addr_high = (addr >> 32) & 0xFFFFFFFFul;
}
static inline uint64_t vidoutgen_set_buf0_offset_addr(volatile vidoutgen_regs_t* p_reg) {
    #ifdef __SYNTHESIS__
    #pragma HLS INLINE
    #endif
    return ((uint64_t)p_reg->buf0_addr_high << 32) + (uint64_t)p_reg->buf0_addr_low;
}

void vidoutgen(
    //vidoutgen_regs_t& regs,
    uint64_t* p_dram,
    uint64_t head_addr,
    uint32_t h
    //hls::stream< hackcpu_video_t >& video_in_stream,
    //hls::stream< vidoutgen_rgb_t >& video_out_stream//,
    //ap_uint<10>& debug_vcounter,
    //ap_uint<1>& debug_data_in,
    //ap_uint<1>& debug_data_out
);
#endif // #ifndef __VIDOUTGEN_HPP__
