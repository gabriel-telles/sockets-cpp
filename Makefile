# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Source files
SRCS_SERVER = src/server.cpp
SRCS_CLIENT = src/client.cpp

# Object files
OBJS_SERVER = $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.cpp=.o)

# Executables
SERVER = server
CLIENT = client

# Default target
all: $(SERVER) $(CLIENT)

# Build server executable
$(SERVER): $(OBJS_SERVER)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_SERVER)

# Build client executable
$(CLIENT): $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_CLIENT)

# Build object files for server
$(OBJS_SERVER): src/server.h

# Build object files for client
$(OBJS_CLIENT): src/client.h

# Clean up
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(SERVER) $(CLIENT)
