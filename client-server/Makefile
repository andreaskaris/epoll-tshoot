.PHONY: client
client:
	gcc client.c -o _output/client

.PHONY: server
server:
	gcc server.c -o _output/server

.PHONY: server_epoll
server_epoll:
	gcc server_epoll.c -o _output/server_epoll

.PHONY: clean
clean:
	rm -f _output/*

.PHONY: build
build: clean client server server_epoll

.PHONY: container-image
container-image:
	podman build -t epoll-client-server .
