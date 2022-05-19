/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>

typedef struct {
    struct sockaddr_in server_address;
    char output_file_name[FILENAME_MAX];
    uint32_t output_size;
} InputArguments;

void parse_input(InputArguments* arguments);
