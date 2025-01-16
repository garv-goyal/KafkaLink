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
        std::cerr << "Failed to create server socket: " << std::endl;
        return 1;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(server_fd);
        std::cerr << "setsockopt failed: " << std::endl;
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
        std::cerr << "Failed to accept connection" << std::endl;
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
    int32_t correlation_id;
    std::memcpy(&correlation_id, buffer + 8, sizeof(correlation_id));
    correlation_id = ntohl(correlation_id); 

    // Extracting request_api_version
    int16_t request_api_version;
    std::memcpy(&request_api_version, buffer + 6, sizeof(request_api_version));
    request_api_version = ntohs(request_api_version);

    std::cout << "Received correlation_id: " << correlation_id << std::endl;
    std::cout << "Received request_api_version: " << request_api_version << std::endl;

    // Find error code
    int16_t error_code = 0; 
    if (request_api_version < 0 || request_api_version > 4) {
        error_code = 35; 
    }

    int32_t message_size = htonl(6); 
    int32_t response_correlation_id = htonl(correlation_id);
    int16_t response_error_code = htons(error_code);

    write(client_fd, &message_size, sizeof(message_size));
    write(client_fd, &response_correlation_id, sizeof(response_correlation_id));
    write(client_fd, &response_error_code, sizeof(response_error_code));

    close(client_fd);
    close(server_fd);
    return 0;
}