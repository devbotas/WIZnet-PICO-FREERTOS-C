#include "rest_client.h"
#include "socket.h"
#include <string.h>
#include <stdio.h>

static int8_t try_send_http_request(uint8_t socket_number, uint32_t server_ip, uint16_t port, const char* method,
                                    const char* path,
                                    const char* body, char* response_buffer, uint16_t response_length) {
    char request[512];
    const uint16_t body_length = body ? strlen(body) : 0;
    uint8_t ip[4];

    ip[0] = (uint8_t)(server_ip >> 24);
    ip[1] = (uint8_t)(server_ip >> 16);
    ip[2] = (uint8_t)(server_ip >> 8);
    ip[3] = (uint8_t)server_ip;

    // 1. Open socket
    int8_t opened_socket_number = socket(socket_number, Sn_MR_TCP, 0, 0);
    if (opened_socket_number != socket_number) {
        goto error;
    }

    // 2. Connect to server
    uint8_t connect_result = connect(socket_number, ip, port);
    if (connect_result != SOCK_OK) {
        goto error;
    }

    // 3. Format request
    if (body_length > 0) {
        snprintf(request, sizeof(request),
                 "%s %s HTTP/1.1\r\n"
                 "Host: %d.%d.%d.%d\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n"
                 "%s",
                 method, path, ip[0], ip[1], ip[2], ip[3], body_length, body);
    }
    else {
        snprintf(request, sizeof(request),
                 "%s %s HTTP/1.1\r\n"
                 "Host: %d.%d.%d.%d\r\n"
                 "Connection: close\r\n\r\n",
                 method, path, ip[0], ip[1], ip[2], ip[3]);
    }

    // 4. Send request
    uint8_t send_result = send(socket_number, (uint8_t*)request, strlen(request));
    if (send_result < 0) {
        goto error;
    }

    // 5. Receive response
    if (response_buffer && response_length > 0) {
        memset(response_buffer, 0, response_length);
        int32_t received_count = 0;
        while (received_count < response_length - 1) {
            const int32_t receive_result = recv(socket_number, (uint8_t*)response_buffer + received_count,
                                                response_length - 1 - received_count);

            if (receive_result == SOCKERR_SOCKCLOSED) break;
            if (receive_result < 0) break;

            received_count += receive_result;
        }
    }

    // 6. Close and return
    disconnect(socket_number);
    close(socket_number);
    return 1;

error:
    close(socket_number);
    return 0;
}

int8_t rest_get(const uint32_t server_ip, const uint16_t port, const char* path, char* response_buffer,
                uint16_t response_length) {
    return try_send_http_request(1, server_ip, port, "GET", path, NULL, response_buffer, response_length);
}

int8_t rest_post(uint32_t server_ip, uint16_t port, const char* path, const char* body,
                 char* response_buffer,
                 uint16_t response_length) {
    return try_send_http_request(2, server_ip, port, "POST", path, body, response_buffer, response_length);
}
