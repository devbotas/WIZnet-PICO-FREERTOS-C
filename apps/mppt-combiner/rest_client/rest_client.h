#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Send a simple HTTP GET request.
 * 
  * @param server_ip IP address of the remote server (in network byte order).
 * @param port Port of the remote server.
 * @param path Path to the resource (e.g., "/api/data").
 * @param response_buffer Buffer to store the response.
 * @param response_length Length of the response buffer.
 * @return int8_t 1 on success, 0 or negative on failure.
 */
int8_t rest_get(uint32_t server_ip, uint16_t port, const char* path, char* response_buffer,
                uint16_t response_length);

/**
 * @brief Send a simple HTTP POST request.
 *
  * @param server_ip IP address of the remote server (in network byte order).
 * @param port Port of the remote server.
 * @param path Path to the resource.
 * @param body POST body data.
 * @param response_buffer Buffer to store the response.
 * @param response_length Length of the response buffer.
 * @return int8_t 1 on success, 0 or negative on failure.
 */
int8_t rest_post(uint32_t server_ip, uint16_t port, const char* path, const char* body,
                 char* response_buffer,
                 uint16_t response_length);


