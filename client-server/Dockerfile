FROM registry.fedoraproject.org/fedora:latest AS builder
RUN mkdir /build
COPY . /build/
WORKDIR /build
RUN yum install "@Development Tools" -y
RUN make server_epoll && make client

FROM registry.fedoraproject.org/fedora-minimal:latest
COPY --from=builder /build/_output/* /usr/bin/
