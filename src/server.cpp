#include "server.h"
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

std::map<int, ClientInfo> client_data;
pthread_mutex_t client_data_lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
    int server_socket;
    struct sockaddr_in server_address;

    setupServerSocket(server_address, server_socket);
    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    while (true) {
        int* client_socket = new int;
        *client_socket = accept(server_socket, nullptr, nullptr);
        if (*client_socket < 0) {
            handleSocketError("Failed to accept connection");
        }

        std::cout << "Accepted new connection" << std::endl;

        pthread_t thread;
        pthread_create(&thread, nullptr, processClientConnection, client_socket);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}

void handleSocketError(const std::string& message) {
    perror(message.c_str());
    exit(EXIT_FAILURE);
}

void setupServerSocket(struct sockaddr_in& address, int& server_socket) {
    int option = 1;

    std::memset(&address, 0, sizeof(address));
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

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        handleSocketError("Failed to bind socket");
    }

    if (listen(server_socket, BACKLOG) < 0) {
        handleSocketError("Failed to listen on socket");
    }
}

void* processClientConnection(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    delete (int*)client_socket_ptr;

    std::cout << "Processing client request" << std::endl;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the message

        std::cout << "Received message: " << buffer << std::endl;

        MessageType message_type = parseMessageType(buffer);
        switch (message_type) {
            case MessageType::Get:
                handleGetRequest(client_socket, buffer + 6); // Adjusted for 'MyGet ' prefix
                break;
            case MessageType::LastAccess:
                handleLastAccess(client_socket);
                break;
            case MessageType::Unknown:
                const char* error_message = "Unknown command";
                write(client_socket, error_message, strlen(error_message));
                break;
        }

        updateLastAccess(client_socket);
    }

    if (bytes_read < 0) {
        perror("Failed to read from socket");
    }

    std::cout << "Closing connection" << std::endl;
    close(client_socket);
    return nullptr;
}

MessageType parseMessageType(const std::string& message) {
    if (message.rfind("MyGet ", 0) == 0) {
        return MessageType::Get;
    } else if (message == "MyLastAccess") {
        return MessageType::LastAccess;
    }
    return MessageType::Unknown;
}

void handleGetRequest(int client_socket, const std::string& file_path) {
    int file_descriptor = open(file_path.c_str(), O_RDONLY);
    if (file_descriptor < 0) {
        const char* error_message = "Failed to open file";
        write(client_socket, error_message, strlen(error_message));
        return;
    }

    std::cout << "Sending file: " << file_path << std::endl;

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

void handleLastAccess(int client_socket) {
    pthread_mutex_lock(&client_data_lock);
    auto it = client_data.find(client_socket);
    if (it != client_data.end()) {
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
    client_data[client_socket].last_access_time = time(nullptr);
    pthread_mutex_unlock(&client_data_lock);
}
