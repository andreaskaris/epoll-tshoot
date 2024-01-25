## Build and run the test server and client

```
make -C client-server/ build
```

In one CLI:
```
client-server/_output/server_epoll
```

In another CLI:
```
client-server/_output/client
```

## Run systemtap on local system

```
make -C systemtap run-stap
```

## Build container image

Build image:
```
make -C client-server container-image-build
```

Push image:
```
make -C client-server container-image-push
```

## Deploy epoll client and server

```
make -C kubernetes deploy
# make -C kubernetes clean # For cleanup.
```

## Deploy systemtap daemonset

```
make -C kubernetes deploy-systemtap SYSTEMTAP_IMAGE="quay.io/akaris/systemtap:ocp-4.12.33-rt" NODE_SELECTOR="worker" PGREP="server_epoll 0.0.0.0 2222|client 172.30.44.140 2222"
# make -C kubernetes clean-systemtap # For cleanup.
```
