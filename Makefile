all: server client

server: gomoku_server.c
	gcc -Wall -Werror -Wextra gomoku_server.c -o server

client: gomoku_client.c
	gcc -Wall -Werror -Wextra gomoku_client.c -o client

clean:
	rm -f server client

rebuild: clean all