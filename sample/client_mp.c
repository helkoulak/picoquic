#include "picoquic.h"
#include "picoquic_internal.h"
#include "picosocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback function to handle events
int client_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length,
                    picoquic_call_back_event_t event, void* callback_ctx, void* stream_ctx) {
    if (event == picoquic_callback_stream_data) {
        printf("Client received data on stream %llu: %.*s\n", stream_id, (int)length, bytes);
    }
    return 0;
}

int main() {
    picoquic_quic_t* client = NULL;
    picoquic_cnx_t* cnx = NULL;
    struct sockaddr_in server_addr;
    int ret;

    // Create client QUIC context
    client = picoquic_create(8, NULL, NULL, NULL, "hq-29", client_callback, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, 0);
    if (client == NULL) {
        fprintf(stderr, "Error creating client context.\n");
        return -1;
    }

    // Enable multipath
    picoquic_set_multipath_enabled_by_default(client, 1);

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4443);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Create a new connection
    cnx = picoquic_create_cnx(client, picoquic_null_connection_id, picoquic_null_connection_id,
                              (struct sockaddr*)&server_addr, picoquic_get_quic_time(client), 0, "localhost", "hq-29", 1);
    if (cnx == NULL) {
        fprintf(stderr, "Error creating client connection.\n");
        picoquic_free(client);
        return -1;
    }

    // Start the connection
    ret = picoquic_start_client_cnx(cnx);
    if (ret != 0) {
        fprintf(stderr, "Error starting client connection.\n");
        picoquic_free(client);
        return -1;
    }

    // Add a second path (simulate multiple interfaces)
    struct sockaddr_in second_addr = server_addr; // Copy server address
    second_addr.sin_port = htons(4444); // Different port for the second path
    int socket2 = picoquic_bind_to_port(client, 5001); // Bind client to a second socket
    if (socket2 < 0) {
        fprintf(stderr, "Error binding second socket.\n");
        picoquic_free(client);
        return -1;
    }

    ret = picoquic_add_path(cnx, (struct sockaddr*)&second_addr, NULL, socket2);
    if (ret != 0) {
        fprintf(stderr, "Error adding second path.\n");
    }

    // Send data
    char message[] = "Hello, multipath QUIC!";
    picoquic_add_to_stream(cnx, 0, (uint8_t*)message, strlen(message), 1);

    // Main event loop
    while (picoquic_get_cnx_state(cnx) != picoquic_state_disconnected) {
        picoquic_packet_loop(client, 1, 1, 0, NULL);
    }

    // Cleanup
    picoquic_free(client);
    return 0;
}
