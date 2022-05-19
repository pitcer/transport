/**
 * Piotr Dobiech 316625
 */

#include "transport.h"
#include "ip_utils.h"

void parse_input(InputArguments* arguments) {
    char server_address_string[20];
    uint16_t server_port;
    scanf("%s %" SCNu16 " %s %" SCNu32, server_address_string, &server_port,
        arguments->output_file_name, &arguments->output_size);
    create_socket_address(server_address_string, server_port, &arguments->server_address);
}
