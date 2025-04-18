# Trace Evaluator

- Input the trace and LTL property,
  we evaluate the correctness of that
  trace by passing the trace into an
  evaluator exposed via [gRPC](./proto/ltlfuzz.proto)
  api implemented upon [LTL-Fuzzer](https://github.com/ltlfuzzer/LTL-Fuzzer)

- [example](./cmd/example)
