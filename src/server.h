#ifndef SERVER_H
#define SERVER_H

void setupServerSocket(struct sockaddr_in &address, int &server_socket);
void handleSocketError(const char *message);
void handleConnectionError(int result, const char *message);
void processClientConnection(int client_socket);

#endif // SERVER_H