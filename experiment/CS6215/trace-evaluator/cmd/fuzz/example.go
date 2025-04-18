package main

import (
	"tracor/cmd/fuzz/event"
)

var quota = 500
var each_req_size = 100

func report_exceed(e *event.EventLogger) {
	e.LogEvent(event.ReportExceeded)
}

func impl(activate_quota bool, req_time int, problematic bool, e *event.EventLogger) {
	if activate_quota {
		e.LogEvent(event.QuotaActivate)
	} else {
		return
	}

	var user_size = 0
	for i := 0; i < req_time; i++ {
		user_size += each_req_size
	}

	if user_size > quota {
		e.LogEvent(event.QuotaExceeded)

		// NOTE: correct implementation
		if !problematic {
			report_exceed(e)
		}
	}

}
