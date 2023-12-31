/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>

#define TICK_EPSILON_SECONDS 0
#define TICK_EPSILON_MICROSECONDS 100

static const struct timeval TICK_EPSILON_TIME
    = { .tv_sec = TICK_EPSILON_SECONDS, .tv_usec = TICK_EPSILON_MICROSECONDS };

#define PACKET_TIMEOUT_SECONDS 0
#define PACKET_TIMEOUT_MICROSECONDS 400000

static const struct timeval PACKET_TIMEOUT_TIME
    = { .tv_sec = PACKET_TIMEOUT_SECONDS, .tv_usec = PACKET_TIMEOUT_MICROSECONDS };

#define PACKET_DATA_MAXUMUM_LENGTH 1000

#define WINDOW_SIZE 3200

// "GET <start> <length>\n"
// <start> \in [0, 10_000_000]
// <length> \in [0, 1_000]
#define REQUEST_PACKET_MAXIMUM_LENGTH (3 + 1 + 8 + 1 + 4 + 1)

// "DATA <start> <length>\n<data>"
// <start> \in [0, 10_000_000]
// <length> \in [0, 1_000]
#define RESPONSE_PACKET_MAXIMUM_LENGTH (4 + 1 + 8 + 1 + 4 + 1 + PACKET_DATA_MAXUMUM_LENGTH)

typedef struct {
    uint32_t start;
    uint16_t length;
} RequestData;

typedef struct {
    uint32_t start;
    uint16_t length;
    const uint8_t* data;
    size_t data_size;
} ResponseData;

size_t create_request_packet(
    const RequestData* request, uint8_t request_packet[REQUEST_PACKET_MAXIMUM_LENGTH]);

void parse_response_packet(const size_t response_packet_length,
    const uint8_t response_packet[RESPONSE_PACKET_MAXIMUM_LENGTH], ResponseData* response);
