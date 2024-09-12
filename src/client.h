#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

enum CommandType {
    MYGET,
    MYLASTACCESS
};

void handleSocketError(int socket_fd);
void initializeSocketAddress(struct sockaddr_in &address, struct hostent *host);
void displayErrorAndExit(const char *message);
void sendRequest(int socket_fd, const char *request);
void receiveResponse(int socket_fd);

#endif // CLIENT_H

