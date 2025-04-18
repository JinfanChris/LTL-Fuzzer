# Example workflow

- Example:

![](https://raw.githubusercontent.com/ChrisVicky/image-bed/main/2025-04/CS6215-example.png)

---

```go
package main

import (
 "tracor/client"

 "github.com/sirupsen/logrus"
)

func main() {
 c, err := client.NewLTLFuzzClient("localhost:50051")
 if err != nil {
  logrus.Fatalf("failed to create client: %v", err)
  panic(err)
 }

 defer c.Close()

 // Unintended LTL Properties
 ltlProperties := []string{"F(a&F(o&G(!n)))", "F(a&F(o&F(n)))"}
 exclude := []string{"a,o,n,b", "a,o,n,b"}
 // exclude := []string{""}
 msg, err := c.PrepareLTL(ltlProperties, exclude)

 if err != nil {
  logrus.Fatalf("failed to prepare LTL properties: %v", err)
  panic(err)
 }

 logrus.Infof("LTL properties prepared: %s", msg)

 logrus.Infof("Input LTL properties: ")
 for _, l := range ltlProperties {
  logrus.Infof("\t- %s", l)
 }

 trace := "a,o,b,b,b,b,b"

 _, violations, err := c.SubmitTrace(trace)
 if err != nil {
  logrus.Fatalf("failed to submit trace: %v", err)
  panic(err)
 }

 logrus.Infof("Input Trace: %s", trace)

 if len(violations) > 0 {
  for _, v := range violations {
   logrus.Infof("Trace satisfied properties: %v", v)
  }
  logrus.Warnf("Trace is INVALID")
 } else {
  logrus.Infof("No satisfied properties, meaning the traces is VALID")
 }
}
```

- output:
  > Please run the server prior to this script. The server is located in `${PROJECT_DIR}/build/src/grpc/ltlfuzz_grpc_server`

```bash
❯ go run cmd/example/main.go
INFO[0000] LTL properties prepared: Properties loaded.
INFO[0000] Input LTL properties:
INFO[0000]      - F(a&F(o&G(!n)))
INFO[0000]      - F(a&F(o&F(n)))
INFO[0000] Input Trace: a,o,b,b,b,b,b
INFO[0000] Trace satisfied properties: F(a & F(o & G!n)),
WARN[0000] Trace is INVALID
```
