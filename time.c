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
    // suseconds_t base_micros = end_time->tv_usec;
    // time_t seconds_to_subtract = start_time->tv_sec;
    // if (start_time->tv_usec > end_time->tv_usec) {
    //     seconds_to_subtract += 1;
    //     base_micros += MICROS_IN_SECOND;
    // }

    // const time_t seconds = end_time->tv_sec - seconds_to_subtract;
    // end_time->tv_usec = base_micros - start_time->tv_usec;

    // end_time->tv_sec = seconds < 0 ? 0 : seconds;
    if (result->tv_sec < 0) {
        timerclear(result);
    }
}

// bool is_less_or_equal(const struct timeval* first, const struct timeval* second) {
//     return (first->tv_sec < second->tv_sec)
//         || (first->tv_sec == second->tv_sec && first->tv_usec <= second->tv_usec);
// }
