CC          = gcc
CFLAGS      = -Wall -Wextra -std=c11
RM          = rm -f
SERVER      = tcp_server.c
SERVER_EXEC = tcp_server
CLIENT      = tcp_client.c
CLIENT_EXEC = tcp_client

all: $(SERVER_EXEC) $(CLIENT_EXEC)

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
