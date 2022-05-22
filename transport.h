/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>

#define MICROS_IN_SECOND 1000000

#define TICK_SECONDS 1
#define TICK_MICROSECONDS 0

#define PACKET_TIMEOUT_SECONDS 2
#define PACKET_TIMEOUT_MICROSECONDS 0

#define PACKET_DATA_MAXUMUM_LENGTH 1000

#define WINDOW_SIZE 1

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
