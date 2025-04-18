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
