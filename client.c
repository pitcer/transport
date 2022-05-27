/**
 * Piotr Dobiech 316625
 */

#include "client.h"

#include "io.h"
#include "time.h"
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

static inline uint32_t get_packets_to_send(const Client* client) {
    return div_ceil(client->output_size, PACKET_DATA_MAXUMUM_LENGTH);
}

void initialize_client(const InputArguments* arguments, Client* client) {
    client->socket_fd = create_socket();
    const int flags = O_WRONLY | O_CREAT | O_APPEND | O_TRUNC;
    const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    client->output_fd = open_file(arguments->output_file_name, flags, mode);
    client->server_address = arguments->server_address;
    client->output_size = arguments->output_size;
    client->written_packets = 0;

    const uint32_t packets_to_send = get_packets_to_send(client);
    const size_t window_size = min(packets_to_send, WINDOW_SIZE);
    initialize_window(window_size, &client->window);
}

static inline uint32_t get_last_packet_length(const Client* client) {
    if (client->output_size == 0) {
        return 0;
    } else if (client->output_size % PACKET_DATA_MAXUMUM_LENGTH == 0) {
        return PACKET_DATA_MAXUMUM_LENGTH;
    } else {
        return client->output_size % PACKET_DATA_MAXUMUM_LENGTH;
    }
}

static inline void send_request(const Client* client, const RequestData* request) {
    uint8_t packet[REQUEST_PACKET_MAXIMUM_LENGTH];
    const size_t packet_length = create_request_packet(request, packet);
    send_to(client->socket_fd, packet, packet_length, &client->server_address);
}

static inline void send_first_request(
    Client* client, const uint32_t start, const struct timeval* now) {

    push_window_entry(&client->window, start, now);
    const RequestData request = { .start = start, .length = PACKET_DATA_MAXUMUM_LENGTH };
    send_request(client, &request);
}

static inline void send_initial_requests(Client* client, struct timeval* select_time) {
    const uint32_t packets_to_send = div_ceil(client->output_size, PACKET_DATA_MAXUMUM_LENGTH);
    const uint32_t size = min(packets_to_send, client->window.size);

    select_time->tv_sec = PACKET_TIMEOUT_SECONDS + TICK_EPSILON_SECONDS;
    select_time->tv_usec = PACKET_TIMEOUT_MICROSECONDS + TICK_EPSILON_MICROSECONDS;

    for (size_t index = 0; index < size; index++) {
        const uint32_t start = PACKET_DATA_MAXUMUM_LENGTH * index;
        struct timeval now;
        get_time(&now);
        send_first_request(client, start, &now);
    }
}

static inline void resend_timeouted_requests(Client* client, struct timeval* select_time) {
    WindowIterator iterator;
    create_window_iterator(&client->window, &iterator);

    struct timeval lowest_timeout_time;
    timerclear(&lowest_timeout_time);

    WindowEntry* entry;
    while ((entry = iterate_window(&iterator)) != NULL) {
        if (!timerisset(&entry->timeout_time)) {
            continue;
        }

        struct timeval now;
        get_time(&now);

        if (timercmp(&entry->timeout_time, &now, <=)) {
            const RequestData request
                = { .start = entry->start, .length = PACKET_DATA_MAXUMUM_LENGTH };
            send_request(client, &request);
            set_timeout_time(entry, &now);
        }

        if (!timerisset(&lowest_timeout_time)
            || timercmp(&entry->timeout_time, &lowest_timeout_time, <)) {
            lowest_timeout_time = entry->timeout_time;
        }
    }

    struct timeval now;
    get_time(&now);
    subtract_to_zero_time(&lowest_timeout_time, &now, select_time);
    timeradd(select_time, &TICK_EPSILON_TIME, select_time);
}

static inline void receive_response(
    const Client* client, uint8_t packet[IP_MAXPACKET], ResponseData* response) {

    struct sockaddr_in sender;

    const size_t packet_length = receive_from(client->socket_fd, packet, IP_MAXPACKET, &sender);
    if (!is_socket_address_equal(&sender, &client->server_address)) {
        eprintln("Received packet from unknown server.");
        exit(EXIT_FAILURE);
    }

    parse_response_packet(packet_length, packet, response);
}

static inline void append_and_pull_prefix_of_received_entries(Client* client) {
    WindowIterator iterator;
    create_window_iterator(&client->window, &iterator);

    const uint32_t packets_to_send = get_packets_to_send(client);

    WindowEntry* entry;
    while ((entry = iterate_window(&iterator)) != NULL) {
        if (timerisset(&entry->timeout_time)) {
            return;
        }

        const size_t buffer_size = client->written_packets + 1 == packets_to_send
            ? get_last_packet_length(client)
            : PACKET_DATA_MAXUMUM_LENGTH;

        write_fd(client->output_fd, entry->data_buffer, buffer_size);
        pull_window_entry(&client->window);
        pull_window_iterator(&iterator);

        if (client->written_packets + client->window.size < packets_to_send) {
            const uint32_t start
                = PACKET_DATA_MAXUMUM_LENGTH * (client->written_packets + client->window.size);
            struct timeval now;
            get_time(&now);
            send_first_request(client, start, &now);
        }

        client->written_packets++;

        const uint32_t percent = (client->written_packets * 100) / packets_to_send;
        printf("\r%6uKB/%uKB - %u%%", client->written_packets, packets_to_send, percent);
        fflush(stdout);
    }
}

static inline void append_output(const Client* client, ResponseData* response) {
    write_fd(client->output_fd, response->data, response->data_size);
}

static inline void handle_received_response(Client* client, struct timeval* select_time) {
    struct timeval start_time;
    get_time(&start_time);

    // Response holds a reference to memory owned by packet.
    uint8_t packet[IP_MAXPACKET];
    ResponseData response;
    receive_response(client, packet, &response);
    debug_assert(response.length == 1000);

    const uint32_t start = response.start;

    WindowEntry* entry = get_window_entry(&client->window, start);
    if (entry == NULL) {
        // Ignore invalid response (if start is not in the window).
        return;
    }
    memcpy(entry->data_buffer, response.data, PACKET_DATA_MAXUMUM_LENGTH);
    timerclear(&entry->timeout_time);

    append_and_pull_prefix_of_received_entries(client);

    struct timeval delta_time;
    get_time(&delta_time);

    timersub(&delta_time, &start_time, &delta_time);
    subtract_to_zero_time(select_time, &delta_time, select_time);
}

void run_client(Client* client) {
    const uint32_t packets_to_send = get_packets_to_send(client);

    struct timeval select_time;
    send_initial_requests(client, &select_time);

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
            resend_timeouted_requests(client, &select_time);
        } else { // ready > 0
            handle_received_response(client, &select_time);

            if (client->written_packets == packets_to_send) {
                println();
                return;
            }
        }
    }
}

void uninitialize_client(Client* client) {
    uninitialize_window(&client->window);
    close_fd(client->socket_fd);
    close_fd(client->output_fd);
}
