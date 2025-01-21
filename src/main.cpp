#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(server_fd);
        std::cerr << "setsockopt failed." << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9092);

    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
        close(server_fd);
        std::cerr << "Failed to bind to port 9092" << std::endl;
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        close(server_fd);
        std::cerr << "listen failed" << std::endl;
        return 1;
    }

    std::cout << "Waiting for a client to connect...\n";

    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    if (client_fd < 0) {
        std::cerr << "Failed to accept a client connection." << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "Client connected\n";

    uint8_t buffer[1024];
    ssize_t received_bytes = read(client_fd, buffer, sizeof(buffer));

    if (received_bytes < 12) { 
        std::cerr << "Invalid request: insufficient data" << std::endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Extracting correlation_id
    int32_t raw_corr_id;
    memcpy(&raw_corr_id, buffer+8, sizeof(raw_corr_id));
    int32_t correlation_id = ntohl(raw_corr_id); 

    // Extracting request_api_version
    int16_t request_api_version;
    memcpy(&request_api_version, buffer + 6, sizeof(request_api_version));
    request_api_version = ntohs(request_api_version);

    std::cout << "Received correlation_id: " << correlation_id << std::endl;
    std::cout << "Received request_api_version: " << request_api_version << std::endl;

    // Find error code
    int16_t error_code = 0; 
    if (request_api_version < 0 || request_api_version > 4) {
        error_code = 35; 
    }

    int16_t network_error_code = htons(error_code);
    uint8_t compact_length = 0x02; 
    int16_t network_api_key = htons(18);
    int16_t network_min_ver = htons(0);
    int16_t network_max_ver = htons(4);
    int32_t network_throttle_time = htonl(0);
    uint8_t tag_count = 0x00;      
    uint8_t entry_tag_count = 0x00; 
    int32_t total_size = htonl(19);
    int32_t network_corr_id = htonl(correlation_id);

    send(client_fd, &total_size, sizeof(total_size), 0);
    send(client_fd, &network_corr_id, sizeof(network_corr_id), 0);
    send(client_fd, &network_error_code, sizeof(network_error_code), 0);
    send(client_fd, &compact_length, sizeof(compact_length), 0);
    send(client_fd, &network_api_key, sizeof(network_api_key), 0);
    send(client_fd, &network_min_ver, sizeof(network_min_ver), 0);
    send(client_fd, &network_max_ver, sizeof(network_max_ver), 0);
    send(client_fd, &entry_tag_count, sizeof(entry_tag_count), 0);
    send(client_fd, &network_throttle_time, sizeof(network_throttle_time), 0);
    send(client_fd, &tag_count, sizeof(tag_count), 0);

    close(client_fd);
    close(server_fd);
    return 0;
}