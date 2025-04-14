all:
	gcc gomoku_server.c -o server
	gcc gomoku_client.c -o client

clean:
	rm -f server client

server:
	./server

client:
	./client 127.0.0.1 8080

clear_80:
	sudo fuser -k 80/tcp