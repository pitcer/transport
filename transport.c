/**
 * Piotr Dobiech 316625
 */

#include "transport.h"

#include "io.h"
#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Returns request packet length.
 */
size_t create_request_packet(
    const RequestData* request, uint8_t request_packet[REQUEST_PACKET_MAXIMUM_LENGTH]) {

    debug_assert(request->start <= 10000000);
    debug_assert(request->length <= 1000);

    // `sprintf` returns number of printed characters excluding null terminator.
    const int result = sprintf(
        (char*)request_packet, "GET %" PRIu32 " %" PRIu16 "\n", request->start, request->length);
    if (result < 0) {
        eprintln("sprintf error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return result;
}

void parse_response_packet(const size_t response_packet_length,
    const uint8_t response_packet[RESPONSE_PACKET_MAXIMUM_LENGTH], ResponseData* response) {

    // %n has problems with handling '\r' character when there is '\n' in format.
    int header_length;
    const int result = sscanf((char*)response_packet, "DATA %" SCNu32 " %" SCNu16 "%n",
        &response->start, &response->length, &header_length);
    if (result == EOF) {
        eprintln("sscanf error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (response_packet[header_length] != '\n') {
        eprintln("Invalid DATA packet format.");
        exit(EXIT_FAILURE);
    }
    header_length++;

    response->data = response_packet + header_length;
    response->data_size = response_packet_length - header_length;

    debug_assert(response->start <= 10000000);
    debug_assert(response->length <= 1000);
}
