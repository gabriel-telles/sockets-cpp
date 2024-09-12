#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "client.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
    int connection_fd;
    int socket_fd;
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE];
    struct hostent *server_host;
    struct sockaddr_in server_address;

    validateArguments(argc);

    server_host = gethostbyname(argv[1]);
    validateHostName(server_host);

    socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    handleSocketError(socket_fd);

    initializeSocketAddress(server_address, server_host);

    connection_fd = connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    handleConnectionError(connection_fd);

    write(socket_fd, argv[2], strlen(argv[2]) + 1);

    while ((bytes_received = read(socket_fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_received);
    }

    if (bytes_received < 0) {
        displayErrorAndExit("Failed to read data from socket");
    }

    close(socket_fd);
    return 0;
}

void initializeSocketAddress(struct sockaddr_in &address, struct hostent *host) {
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    memcpy(&address.sin_addr.s_addr, host->h_addr, host->h_length);
    address.sin_port = htons(SERVER_PORT);
}

void validateHostName(struct hostent *host) {
    if (!host) {
        displayErrorAndExit("Failed to resolve host name");
    }
}

void handleConnectionError(int connection_status) {
    if (connection_status < 0) {
        displayErrorAndExit("Failed to connect to server");
    }
}

void handleSocketError(int socket_fd) {
    if (socket_fd < 0) {
        displayErrorAndExit("Socket creation failed");
    }
}

void validateArguments(int argc) {
    if (argc != 3) {
        displayErrorAndExit("Usage: client server-name file-name");
    }
}

void displayErrorAndExit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}
