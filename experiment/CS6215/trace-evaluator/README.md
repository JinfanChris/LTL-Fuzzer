# Trace Evaluator

- Input the trace and LTL property, we evaluate the correctness of that trace by passing the trace into a grpc api implemented based on LTL Fuzzer 


- example:
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

	ltlProperties := []string{"!G (a -> F c)", "!G (b -> F !a)"}
	msg, err := c.PrepareLTL(ltlProperties)

	if err != nil {
		logrus.Fatalf("failed to prepare LTL properties: %v", err)
		panic(err)
	}

	logrus.Infof("LTL properties prepared: %s", msg)

	logrus.Infof("Input LTL properties: ")
	for _, l := range ltlProperties {
		logrus.Infof("\t- %s", l)
	}

	trace := "a,b,c,b"

	_, violations, err := c.SubmitTrace(trace)
	if err != nil {
		logrus.Fatalf("failed to submit trace: %v", err)
		panic(err)
	}

	logrus.Infof("Input Trace: %s", trace)

	if len(violations) > 0 {
		for _, v := range violations {
			logrus.Infof("Trace violates properties: %v", v)
		}
	} else {
		logrus.Infof("Trace satisfies all properties")
	}
}
```
- output: 
```bash
❯ go run ./main.go
INFO[0000] LTL properties prepared: Properties loaded.
INFO[0000] Input LTL properties:
INFO[0000]      - !G (a -> F c)
INFO[0000]      - !G (b -> F !a)
INFO[0000] Input Trace: a,b,c,b
INFO[0000] Trace violates properties: !G (a -> F c),
```


