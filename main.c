/**
 * Piotr Dobiech 316625
 */

#include "client.h"
#include "io.h"
#include "utils.h"
#include <stdlib.h>

static inline void parse_input(int argc, char const* argv[], InputArguments* arguments) {
    if (argc != 5) {
        println("Usage: transport <server address> <server port> <output file> <bytes size>");
        exit(EXIT_FAILURE);
    }

    const char* server_address_string = argv[1];
    uint16_t server_port = strtoumax(argv[2], NULL, 10);
    create_socket_address(server_address_string, server_port, &arguments->server_address);

    arguments->output_file_name = argv[3];
    arguments->output_size = strtoumax(argv[4], NULL, 10);

    if (arguments->output_size == 0) {
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char const* argv[]) {
    InputArguments arguments;
    parse_input(argc, argv, &arguments);

    Client client;
    initialize_client(&arguments, &client);
    run_client(&client);
    uninitialize_client(&client);

    return EXIT_SUCCESS;
}
