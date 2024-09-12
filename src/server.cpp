#include "server.h"
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

std::map<int, ClientInfo> client_data_mutex;
pthread_mutex_t client_data_lock = PTHREAD_MUTEX_INITIALIZER;


int main() {
    int server_socket;
    struct sockaddr_in server_address;

    setupServerSocket(server_address, server_socket);

    while (1) {
        int *client_socket = new int;
        *client_socket = accept(server_socket, nullptr, nullptr);
        if (*client_socket < 0) {
            handleSocketError("Failed to accept connection");
        }

        pthread_t thread;
        pthread_create(&thread, nullptr, processClientConnection, client_socket);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}

void handleSocketError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
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

void* processClientConnection(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    delete (int*)client_socket_ptr;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the message

        MessageType message_type = parseMessageType(buffer);
        if (message_type == MYGET) {
            handleMyGet(client_socket, buffer + 4);  // Assuming 'MyGet ' prefix
        } else if (message_type == MYLASTACCESS) {
            handleMyLastAccess(client_socket);
        } else {
            const char* error_message = "Unknown command";
            write(client_socket, error_message, strlen(error_message));
        }

        updateLastAccess(client_socket);
    }

    close(client_socket);
    return nullptr;
}

MessageType parseMessageType(const char *message) {
    if (strncmp(message, "MyGet ", 6) == 0) {
        return MYGET;
    } else if (strncmp(message, "MyLastAccess", 12) == 0) {
        return MYLASTACCESS;
    }
    return static_cast<MessageType>(-1);
}

void handleMyGet(int client_socket, const char *file_path) {
    int file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor < 0) {
        const char* error_message = "Failed to open file";
        write(client_socket, error_message, strlen(error_message));
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_descriptor, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_socket, buffer, bytes_read) < 0) {
            perror("Failed to write to socket");
            break;
        }
    }

    if (bytes_read < 0) {
        perror("Failed to read from file");
    }

    close(file_descriptor);
}

void handleMyLastAccess(int client_socket) {
    pthread_mutex_lock(&client_data_lock);
    auto it = client_data_mutex.find(client_socket);
    if (it != client_data_mutex.end()) {
        time_t last_access_time = it->second.last_access_time;
        char time_buffer[64];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&last_access_time));
        write(client_socket, time_buffer, strlen(time_buffer));
    } else {
        const char* response = "Last Access=Null";
        write(client_socket, response, strlen(response));
    }
    pthread_mutex_unlock(&client_data_lock);
}

void updateLastAccess(int client_socket) {
    pthread_mutex_lock(&client_data_lock);
    client_data_mutex[client_socket].last_access_time = time(nullptr);
    pthread_mutex_unlock(&client_data_lock);
}
