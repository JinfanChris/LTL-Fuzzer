# Fuzz test framework

## How to run

```bash
go test
```

## How to fuzz

```bash
go test -fuzz=Fuzz -fuzztime=10s
```

> Currently, the fuzz test is not working due to extensive request via gRPC makes the server side crash due to some weird issues. I believe this is mainly caused by dirty write issues as multiple tests are running in parallel. I will try to fix this issue later.
