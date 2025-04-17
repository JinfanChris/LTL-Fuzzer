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

	msg, err := c.PrepareLTL([]string{"G (a -> F b)", "G (b -> F a)"})
	if err != nil {
		logrus.Fatalf("failed to prepare LTL properties: %v", err)
		panic(err)
	}

	logrus.Infof("LTL properties prepared: %s", msg)

	satisfied, violations, err := c.SubmitTrace("a b a b")
	if err != nil {
		logrus.Fatalf("failed to submit trace: %v", err)
		panic(err)
	}

	logrus.Infof("Trace satisfied properties: %v, violations: %v", satisfied, violations)
}
