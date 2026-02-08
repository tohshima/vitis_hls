#include <cstdint>
#include <ap_int.h>
#include <hls_stream.h>
#include "vidoutgen.hpp"
#include "gif.h" // https://github.com/charlietangora/gif-h.git
#include "video_stream.hpp"

static uint64_t video_frame[MAX_WIDTH*MAX_HEIGHT*sizeof(vidoutgen_rgba_t)/sizeof(uint64_t)+0x1000];
static hls::stream< hackcpu_video_t > video_in_stream;


int main() {

    vidoutgen_regs_t regs;
    vidoutgen_quick_test(&regs, video_frame, video_in_stream);
    // Just CLS was executed in case of simulation

    // Create a gif
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t delay = 2;
    int32_t bitDepth = 8;
    bool dither = false;
    GifWriter writer = {};
    GifBegin( &writer, "vidoutgen.gif", width, height, delay, bitDepth, dither );
    // Convert Initial display
    GifWriteFrame( &writer, (const uint8_t*)video_frame, width, height, delay, bitDepth, dither );

    // make gif animation
    const int num_of_frames = 600;
    const int num_of_stream_input_per_frame = stream_data_size / num_of_frames;

    int stream_index = 0;
    for (int f = 0; f < num_of_frames; f++) {
        for (int s = 0; s < num_of_stream_input_per_frame; s++) {
            video_in_stream.write(stream_data[stream_index++]);
            vidoutgen_call_top(&regs, video_frame, video_in_stream);
        }
        GifWriteFrame( &writer, (const uint8_t*)video_frame, width, height, delay, bitDepth, dither );
        std::cout << "Output #" << f << " frame. si = " << stream_index << "." << std::endl;
    }
    const int residual_frames = stream_data_size - num_of_frames * num_of_stream_input_per_frame;
    if (residual_frames > 0) {
        for (int s = 0; s < residual_frames; s++) {
            video_in_stream.write(stream_data[stream_index++]);
            vidoutgen_call_top(&regs, video_frame, video_in_stream);
        }
        GifWriteFrame( &writer, (const uint8_t*)video_frame, width, height, delay, bitDepth, dither );
        std::cout << "Output #" << num_of_frames-1 << " frame. si = " << stream_index << "." << std::endl;
    }

    return 0;
}