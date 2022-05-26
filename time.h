/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <stdbool.h>
#include <sys/time.h>

void get_time(struct timeval* time);

void subtract_to_zero_time(
    const struct timeval* end_time, const struct timeval* start_time, struct timeval* result);
