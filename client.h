/**
 * Piotr Dobiech 316625
 */

#pragma once

#include "transport.h"
#include "window.h"

typedef struct {
    struct sockaddr_in server_address;
    const char* output_file_name;
    uint32_t output_size;
} InputArguments;

typedef struct {
    int socket_fd;
    int output_fd;
    uint32_t output_size;
    struct sockaddr_in server_address;
    Window window;
    uint32_t written_packets;
} Client;

void initialize_client(const InputArguments* arguments, Client* client);

void run_client(Client* client);

void uninitialize_client(Client* client);
