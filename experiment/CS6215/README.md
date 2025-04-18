# CS6215 Project Code

- The purpose of this experiment is to demonstrate the understanding
  of [LTL-Fuzzer](https://github.com/ltlfuzzer/LTL-Fuzzer) as part of
  the project for [CS6215-Program Analysis](https://nusmods.com/courses/CS6215/advanced-topics-in-program-analysis)

## Trace Generation

- The traces are generated via [Go Fuzzing](https://go.dev/doc/security/fuzz/) on
  a not-yet-open-sourced satellite edge computing emulator
  (examples of such emulator includes [celestial](https://github.com/OpenFogStack/celestial),
  [StarryNet](https://github.com/SpaceNetLab/StarryNet)
  and [OpenSN](https://github.com/OpenSN-Library).
- Below is an example showcasing the generation of traces.
- [ ] Put a GIF showcasing the process. (with Fuzzing on individual modules as
      well as some transactions)

## Trace Evaluator

- [Trace Evaluator](./trace-evaluator/) is implemented via
  - (1) a client reading from the trace file generated via trace generation and
    a LTL property lists to be checked
    > Note: the LTL property here is the _Intended Behaviour_
  - (2) a server that
    1. Transfer the _Negation_ of the LTL property into
       Buchi Automaton _Unintended Behaviour_
    2. Checkes the traces against the Buchi Automaton.
       If accepted, the trace is a _Violation_ to the
       _Intended Behaviour_ specified by the LTL Property.
