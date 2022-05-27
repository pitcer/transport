/**
 * Piotr Dobiech 316625
 */

#include "time.h"

#include "utils.h"
#include <stdlib.h>

#define MICROS_IN_SECOND 1000000

void get_time(struct timeval* time) {
    const int result = gettimeofday(time, NULL);
    if (result != 0) {
        eprintln("gettimeofday: error");
        exit(EXIT_FAILURE);
    }
}

void subtract_to_zero_time(
    const struct timeval* end_time, const struct timeval* start_time, struct timeval* result) {

    timersub(end_time, start_time, result);

    if (result->tv_sec < 0) {
        timerclear(result);
    }
}
