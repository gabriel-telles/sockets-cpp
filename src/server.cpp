#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define BACKLOG 10

int main() {
    int server_socket;
    struct sockaddr_in server_address;

    setupServerSocket(server_address, server_socket);

    while (1) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            handleSocketError("Failed to accept connection");
        }
        processClientConnection(client_socket);
    }

    close(server_socket);
    return 0;
}

void setupServerSocket(struct sockaddr_in &address, int &server_socket) {
    int option = 1;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT);

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        handleSocketError("Failed to create socket");
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        handleSocketError("Failed to set socket options");
    }

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        handleSocketError("Failed to bind socket");
    }

    if (listen(server_socket, BACKLOG) < 0) {
        handleSocketError("Failed to listen on socket");
    }
}

void handleSocketError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void processClientConnection(int client_socket) {
    char buffer[BUFFER_SIZE];
    int file_descriptor;
    ssize_t bytes_read;

    ssize_t file_name_length = read(client_socket, buffer, BUFFER_SIZE);
    if (file_name_length <= 0) {
        close(client_socket);
        return;
    }

    file_descriptor = open(buffer, O_RDONLY);
    if (file_descriptor < 0) {
        perror("Failed to open file");
        close(client_socket);
        return;
    }

    while ((bytes_read = read(file_descriptor, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_socket, buffer, bytes_read) < 0) {
            perror("Failed to write to socket");
            close(file_descriptor);
            close(client_socket);
            return;
        }
    }

    if (bytes_read < 0) {
        perror("Failed to read from file");
    }

    close(file_descriptor);
    close(client_socket);
}
