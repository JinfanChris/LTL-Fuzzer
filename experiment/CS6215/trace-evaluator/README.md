# Trace Evaluator

- Inputs:
  1. the `trace` generated in [`trace-generator`](../trace-generator/)
  2. LTL property (e.g. `F(a&F(o&G(!n)))`)

> note the input LTL property is an Undesired behaviour
> the acceptance of the trace $T$ by an LTL property $\phi$
> indicates trace $T$ violates the specification of property $\phi$.

- we evaluate the correctness of that
  trace by passing the trace into an
  evaluator exposed via [gRPC](./proto/ltlfuzz.proto)
  api implemented upon [LTL-Fuzzer](https://github.com/ltlfuzzer/LTL-Fuzzer)

- [./cmd/example](./cmd/example) an example workflow
