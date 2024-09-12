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

int main(int argc, char** argv) {
    if (argc < 3) {
        displayErrorAndExit("Usage: client <server-name> <command> [file-name]");
    }

    const char* server_name = argv[1];
    const char* command = argv[2];
    const char* file_name = (argc == 4) ? argv[3] : nullptr;

    struct hostent* server_host = gethostbyname(server_name);
    if (!server_host) {
        displayErrorAndExit("Failed to resolve host name");
    }

    int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    handleSocketError(socket_fd);

    struct sockaddr_in server_address;
    initializeSocketAddress(server_address, server_host);

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        displayErrorAndExit("Failed to connect to server");
    }

    string request;
    if (strcmp(command, "MyGet") == 0 && file_name) {
        request = "MyGet " + string(file_name);
    } else if (strcmp(command, "MyLastAccess") == 0) {
        request = "MyLastAccess";
    } else {
        displayErrorAndExit("Invalid command or missing file name");
    }
    sendRequest(socket_fd, request);

    receiveResponse(socket_fd);

    close(socket_fd);
    return 0;
}

void displayErrorAndExit(const std::string& message) {
    perror(message.c_str());
    exit(EXIT_FAILURE);
}

void handleSocketError(int socket_fd) {
    if (socket_fd < 0) {
        displayErrorAndExit("Socket creation failed");
    }
}

void initializeSocketAddress(struct sockaddr_in& address, const struct hostent* host) {
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    memcpy(&address.sin_addr.s_addr, host->h_addr, host->h_length);
    address.sin_port = htons(SERVER_PORT);
}

void sendRequest(int socket_fd, const std::string& request) {
    if (write(socket_fd, request.c_str(), request.size()) < 0) {
        displayErrorAndExit("Failed to send request");
    }
}

void receiveResponse(int socket_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (true) {
        bytes_received = read(socket_fd, buffer, sizeof(buffer) - 1);
        if (bytes_received < 0) {
            displayErrorAndExit("Failed to read data from socket");
        } else if (bytes_received == 0) {
            break; // End of stream, connection closed
        }

        buffer[bytes_received] = '\0';
        if (write(STDOUT_FILENO, buffer, bytes_received) < 0) {
            displayErrorAndExit("Failed to write data to stdout");
        }
        fflush(stdout);
    }
}
