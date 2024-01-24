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
