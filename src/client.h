#ifndef CLIENT_H
#define CLIENT_H

void validateArguments(int argc);
void handleSocketError(int socket_fd);
void handleConnectionError(int connection_status);
void validateHostName(struct hostent *host);
void initializeSocketAddress(struct sockaddr_in &address, struct hostent *host);
void displayErrorAndExit(const char *message);

#endif // CLIENT_H
