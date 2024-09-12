#include "client.h"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

void handleSocketError(int socket_fd) {
    if (socket_fd < 0) {
        displayErrorAndExit("Socket creation failed");
    }
}

void initializeSocketAddress(struct sockaddr_in &address, struct hostent *host) {
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    memcpy(&address.sin_addr.s_addr, host->h_addr, host->h_length);
    address.sin_port = htons(SERVER_PORT);
}

void displayErrorAndExit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void sendRequest(int socket_fd, const char *request) {
    if (write(socket_fd, request, strlen(request)) < 0) {
        displayErrorAndExit("Failed to send request");
    }
}

void receiveResponse(int socket_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = read(socket_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_received) < 0) {
            displayErrorAndExit("Failed to write data to stdout");
        }
    }

    if (bytes_received < 0) {
        displayErrorAndExit("Failed to read data from socket");
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        displayErrorAndExit("Usage: client server-name command [file-name]");
    }

    int connection_fd;
    int socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;

    server_host = gethostbyname(argv[1]);
    if (!server_host) {
        displayErrorAndExit("Failed to resolve host name");
    }

    socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    handleSocketError(socket_fd);

    initializeSocketAddress(server_address, server_host);

    connection_fd = connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connection_fd < 0) {
        displayErrorAndExit("Failed to connect to server");
    }

    if (strcmp(argv[2], "MyGet") == 0 && argc == 4) {
        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "MyGet %s", argv[3]);
        sendRequest(socket_fd, request);
    } else if (strcmp(argv[2], "MyLastAccess") == 0) {
        sendRequest(socket_fd, "MyLastAccess");
    } else {
        displayErrorAndExit("Invalid command or missing file name");
    }

    receiveResponse(socket_fd);
    close(socket_fd);
    return 0;
}
