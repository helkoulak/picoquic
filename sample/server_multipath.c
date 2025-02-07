#include "picoquic.h"
#include "picoquic_internal.h"
#include "picosocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback function to handle events
int server_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length,
                    picoquic_call_back_event_t event, void* callback_ctx, void* stream_ctx) {
    if (event == picoquic_callback_stream_data) {
        printf("Server received data on stream %llu: %.*s\n", stream_id, (int)length, bytes);

        // Echo the data back
        picoquic_add_to_stream(cnx, stream_id, bytes, length, 1);
    }
    return 0;
}

int main() {
    const char* cert_file = "cert.pem";
    const char* key_file = "key.pem";

    picoquic_quic_t* server = NULL;
    picoquic_cnx_t* cnx = NULL;
    int ret;

    // Create server QUIC context
    server = picoquic_create(8, cert_file, key_file, NULL, "hq-29", server_callback, NULL, NULL, NULL, NULL, picoquic_current_time(), NULL, NULL, NULL, 0);
    if (server == NULL) {
        fprintf(stderr, "Error creating server context.\n");
        return -1;
    }

    // Enable multipath
    picoquic_set_multipath_enabled_by_default(server, 1);

    // Bind to a port
    int port = 4443;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int server_socket = picoquic_bind_to_port(server, port);
    if (server_socket < 0) {
        fprintf(stderr, "Error binding server to port %d.\n", port);
        picoquic_free(server);
        return -1;
    }
    printf("Server listening on port %d...\n", port);

    // Event loop
    while (1) {
        picoquic_packet_loop(server, 1, 1, server_socket, NULL);
    }

    picoquic_free(server);
    return 0;
}

