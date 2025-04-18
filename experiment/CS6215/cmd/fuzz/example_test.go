package main

import (
	"fmt"
	"testing"
	"tracor/client"
	"tracor/cmd/fuzz/event"
)

func FuzzImpl(f *testing.F) {
	f.Add(true, 10, false)
	f.Add(true, 10, true) // Problematic input

	// Set up client
	c, err := client.NewLTLFuzzClient("localhost:50051")
	if err != nil {
		f.Fatalf("failed to create client: %v", err)
		panic(err)
	}
	defer c.Close()

	// Unintended LTL Properties
	// NOTE:
	// 	a: activate quota,
	// 	o: quota exceeded,
	// 	n: exceeded is reported
	// 	meaning:
	// 		when quota is eventually activated,
	// 		quota is eventually exceeded,
	// 		it is NEVER reported <- unintended behaviour
	ltlProperties := []string{"F(a&F(o&G(!n)))"}

	// NOTE:
	// 	This is to tell automata that
	// 	These properties are NOT showing at the same time.
	exclude := []string{"a,o,n,b"}

	// Call the Server to load these properties
	msg, uuid, err := c.PrepareLTL(ltlProperties, exclude)
	if err != nil {
		f.Fatalf("failed to prepare LTL properties: %v", err)
		panic(err)
	}
	f.Logf("LTL properties prepared: %s", msg)

	// Load the event logger
	event.Init()

	// Start Fuzzing
	f.Fuzz(func(t *testing.T, activate_quota bool, req_time int, problematic bool) {

		traceFile := fmt.Sprintf("trace_%t_%d_%t.txt", activate_quota, req_time, problematic)

		e := event.NewEventLogger(traceFile)

		e.Start()

		// NOTE:
		// this is the Business Logic
		// events are logged by 'e'
		// We avoid using global vairables to log to
		// prevent conflicts among multiple fuzzing instances
		impl(activate_quota, req_time, problematic, e)

		e.Stop()

		// Load traces
		eventTrace, err := event.LoadTrace(e.FileName())
		if err != nil {
			t.Fatalf("failed to load trace: %v", err)
		}

		_, violations, err := c.SubmitTrace(eventTrace, uuid)
		if err != nil {
			t.Fatalf("failed to submit trace: %v", err)
		}

		t.Logf("Input: %t, %d, %t", activate_quota, req_time, problematic)
		t.Logf("Trace: %s", eventTrace)

		if len(violations) > 0 {
			for _, v := range violations {
				t.Logf("Trace satisfied properties: %v", v)
			}
			t.Fatalf("Trace is INVALID")
		} else {
			t.Logf("No satisfied properties, meaning the traces is VALID")
		}

	})
}
