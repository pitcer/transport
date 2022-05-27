/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#define println(format, ...) printf(format "\n" __VA_OPT__(, ) __VA_ARGS__)

#define eprintln(format, ...) fprintf(stderr, format "\n" __VA_OPT__(, ) __VA_ARGS__)

// #define DEBUG
#ifdef DEBUG
#define debug(...) println(__VA_ARGS__)
#else
#define debug(...)
#endif

#ifdef DEBUG
#define debug_assert(expression) assert(expression)
#else
#define debug_assert(expression)
#endif

void print_as_bytes(const unsigned char* buff, ssize_t length);

uint32_t div_ceil(const uint32_t divident, const uint32_t divisor);

uint32_t mod(const int32_t divident, const uint32_t divisor);

uint32_t min(const uint32_t first, const uint32_t second);
