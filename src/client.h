#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

constexpr int SERVER_PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

enum class CommandType {
    Get,
    LastAccess
};

void handleSocketError(int socket_fd);
void initializeSocketAddress(struct sockaddr_in& address, const struct hostent* host);
void displayErrorAndExit(const std::string& message);
void sendRequest(int socket_fd, const std::string& request);
void receiveResponse(int socket_fd);

#endif // CLIENT_H

