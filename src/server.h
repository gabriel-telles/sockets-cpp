#pragma once

void setupServerSocket(struct sockaddr_in &address, int &server_socket);
void handleSocketError(int socket_fd, const char *message);
void handleConnectionError(int result, const char *message);
void processClientConnection(int client_socket);