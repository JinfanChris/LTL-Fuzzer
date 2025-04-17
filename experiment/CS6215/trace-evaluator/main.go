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
