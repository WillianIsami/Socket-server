CC          = gcc
CFLAGS      = -Wall -Wextra -std=c11
RM          = rm -f
BUILD_DIR 	= build
SERVER      = tcp_server.c
CLIENT      = tcp_client.c
SERVER_EXEC = $(BUILD_DIR)/tcp_server
CLIENT_EXEC = $(BUILD_DIR)/tcp_client

all: $(BUILD_DIR) $(SERVER_EXEC) $(CLIENT_EXEC) 

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER_EXEC): $(SERVER)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_EXEC): $(CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

run_server: $(SERVER_EXEC)
	./$(SERVER_EXEC)

run_client: $(CLIENT_EXEC)
	./$(CLIENT_EXEC)

.PHONY: all run_server run_client clean

clean:
	$(RM) $(SERVER_EXEC) $(CLIENT_EXEC)
