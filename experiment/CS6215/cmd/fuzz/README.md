# Fuzz test framework

## How to run

1. Start grpc server (see)

```bash
# in ${PPROJECT_DIR}/build
./src/grpc/ltlfuzz_grpc_server
```

2. Start Testing

```bash
go test
```

## How to fuzz

```bash
go test -fuzz=Fuzz -fuzztime=10s
```

> Currently, the fuzz test is not working due to extensive requests via gRPC which crashes the server side due to some weird issues. I believe this is mainly caused by dirty write as multiple tests are running in parallel and the server does not have a lock to gurantee the consistency. Will be fixed later.
