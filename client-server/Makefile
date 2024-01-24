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

.PHONY: container-image-build
container-image-build:
	podman build -t quay.io/akaris/epoll-client-server .

.PHONY: container-image-push
container-image-push:
	podman push quay.io/akaris/epoll-client-server
