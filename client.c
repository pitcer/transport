/**
 * Piotr Dobiech 316625
 */

#include "client.h"

#include "io.h"
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

void initialize_client(const InputArguments* arguments, Client* client) {
    client->socket_fd = create_socket();
    const int flags = O_WRONLY | O_CREAT | O_APPEND | O_TRUNC;
    const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    client->output_fd = open_file(arguments->output_file_name, flags, mode);
    client->server_address = arguments->server_address;
    client->output_size = arguments->output_size;
    initialize_window(WINDOW_SIZE, &client->window);
}

static inline void send_request(const Client* client, const RequestData* request) {
    uint8_t packet[REQUEST_PACKET_MAXIMUM_LENGTH];
    const size_t packet_length = create_request_packet(request, packet);
    send_to(client->socket_fd, packet, packet_length, &client->server_address);
}

static inline void receive_response(const Client* client, ResponseData* response) {
    uint8_t packet[IP_MAXPACKET];
    struct sockaddr_in sender;

    const size_t packet_length = receive_from(client->socket_fd, packet, sizeof(packet), &sender);
    if (sender.sin_addr.s_addr != client->server_address.sin_addr.s_addr
        || sender.sin_port != client->server_address.sin_port) {
        eprintln("Received packet from unknown server.");
        exit(EXIT_FAILURE);
    }

    parse_response_packet(packet_length, packet, response);
}

static inline void append_output(const Client* client, ResponseData* response) {
    write_fd(client->output_fd, response->data, response->data_size);
}

static inline void handle_tick(Client* client) {
    WindowIterator iterator;
    create_window_iterator(&client->window, &iterator);

    debug("%lu %lu %lu", client->window.entries_size, client->window.head_position,
        client->window.tail_position);
    WindowEntry* entry;
    while ((entry = iterate_mut_window(&iterator)) != NULL) {
        debug("%u %u", entry->start, entry->length);
        entry->elapsed_ticks++;

        const size_t elapsed_time = (size_t)MICROS_IN_SECOND * entry->elapsed_ticks * TICK_SECONDS
            + (size_t)entry->elapsed_ticks * TICK_MICROSECONDS;
        const size_t timeout_time
            = (size_t)MICROS_IN_SECOND * PACKET_TIMEOUT_SECONDS + PACKET_TIMEOUT_MICROSECONDS;

        debug("%lu %lu", elapsed_time, timeout_time);

        if (elapsed_time >= timeout_time) {
            entry->elapsed_ticks = 0;
            const RequestData request = { .start = entry->start, .length = entry->length };
            send_request(client, &request);
        }
    }
}

void run_client(Client* client) {
    const uint32_t packets_to_send = div_ceil(client->output_size, PACKET_DATA_MAXUMUM_LENGTH);
    const uint32_t last_packet_length = client->output_size % PACKET_DATA_MAXUMUM_LENGTH;
    // uint32_t sent_packets = 0;
    // uint32_t window_position = 0;

    // TODO: support bigger window size
    {
        uint16_t length = packets_to_send == 1 ? last_packet_length : PACKET_DATA_MAXUMUM_LENGTH;
        push_window_entry(&client->window, 0, length);
        const RequestData request = { .start = 0, .length = length };
        send_request(client, &request);
    }

    struct timeval select_time = { .tv_sec = TICK_SECONDS, .tv_usec = TICK_MICROSECONDS };

    while (1) {
        fd_set select_descriptors;
        FD_ZERO(&select_descriptors);
        FD_SET(client->socket_fd, &select_descriptors);

        const int ready
            = select(client->socket_fd + 1, &select_descriptors, NULL, NULL, &select_time);

        if (ready < 0) {
            eprintln("select error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        } else if (ready == 0) {
            debug("1");
            handle_tick(client);

            select_time.tv_sec = TICK_SECONDS;
            select_time.tv_usec = TICK_MICROSECONDS;
        } else { // ready > 0
            debug("2");
            ResponseData response;
            receive_response(client, &response);

            println("Start: %" PRIu32, response.start);
            println("Length: %" PRIu16, response.length);
            println("Data size: %" PRIu64, response.data_size);
            append_output(client, &response);
            break;
        }
    }
}

void uninitialize_client(Client* client) {
    uninitialize_window(&client->window);
    close_fd(client->socket_fd);
    close_fd(client->output_fd);
}
