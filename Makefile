CC          = gcc
CFLAGS      = -Wall -Wextra -std=c11
RM          = rm -f
SERVER      = server.c
SERVER_EXEC = server
CLIENT      = client.c
CLIENT_EXEC = client


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
