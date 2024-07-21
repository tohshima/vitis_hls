#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <hls_stream.h>
#include "hackcpu.hpp" // Assuming the CPU function is in a file named cpu.h

int main() {
    // Debug signals for monitoring
    static ap_uint<1> write_out;
    static word_t outM;
    static addr_t addressM;
    static word_t pc;
    static word_t debug_A;
    static word_t debug_D;
    static word_t debug_instruction;

    cpu_wrapper(write_out, outM, addressM, pc, debug_A, debug_D, debug_instruction);
    return 0;
}