// AXI rregister IF module for hackcpu
#include "hackcpu.hpp"
#include "axireg_if.hpp"

typedef struct {
    ap_uint<32> uart_enable;
} tRegs;

static tRegs sRegs = {
    .uart_enable = 0,
};

ap_uint<32> axireg_get_uart_enable(void) {
    return sRegs.uart_enable;
}

