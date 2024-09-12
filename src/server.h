#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctime>
#include <map>
#include <string>

constexpr int SERVER_PORT = 8080;
constexpr int BUFFER_SIZE = 4096;
constexpr int BACKLOG = 10;

enum class MessageType {
    Get,
    LastAccess,
    Unknown
};

struct ClientInfo {
    time_t last_access_time;
};

void handleSocketError(const std::string& message);
void setupServerSocket(struct sockaddr_in& address, int& server_socket);
void* processClientConnection(void* client_socket_ptr);
MessageType parseMessageType(const std::string& message);
void handleGetRequest(int client_socket, const std::string& file_path);
void handleLastAccess(int client_socket);
void updateLastAccess(int client_socket);

extern std::map<int, ClientInfo> client_data;
extern pthread_mutex_t client_data_lock;

#endif // SERVER_H
