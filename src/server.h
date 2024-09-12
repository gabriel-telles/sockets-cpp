#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctime>
#include <map>
#include <string>

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define BACKLOG 10

enum MessageType {
    MYGET,
    MYLASTACCESS
};

struct ClientInfo {
    time_t last_access_time;
};

void handleSocketError(const char *message);
void setupServerSocket(struct sockaddr_in &address, int &server_socket);
void* processClientConnection(void* client_socket_ptr);
MessageType parseMessageType(const char *message);
void handleMyGet(int client_socket, const char *file_path);
void handleMyLastAccess(int client_socket);
void updateLastAccess(int client_socket);

extern std::map<int, ClientInfo> client_data_mutex;
extern pthread_mutex_t client_data_lock;

#endif // SERVER_H
