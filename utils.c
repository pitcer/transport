/**
 * Piotr Dobiech 316625
 */

#include "utils.h"

#include <inttypes.h>

void print_as_bytes(const unsigned char* buff, ssize_t length) {
    for (ssize_t i = 0; i < length; i++, buff++) {
        printf("%.2x ", *buff);
    }
    println();
}

uint32_t div_ceil(const uint32_t divident, const uint32_t divisor) {
    return 1 + (divident - 1) / divisor;
}

uint32_t mod(const int32_t divident, const uint32_t divisor) {
    const int32_t remainder = divident % divisor;
    return remainder < 0 ? remainder + divisor : (uint32_t)remainder;
}

uint32_t min(const uint32_t first, const uint32_t second) {
    return first < second ? first : second;
}
