package event

import (
	"fmt"
	"os"
	"path/filepath"
	"sync"

	"github.com/sirupsen/logrus"
)

var savedir = "trace"

type EventLogger struct {
	fn     string      // filename
	e_chan chan string // event channel
	wg     sync.WaitGroup
	file   *os.File // file handle
}

var globalLogger *EventLogger

func Init() {
	os.RemoveAll(savedir)
	os.MkdirAll(savedir, os.ModePerm)
}

func NewEventLogger(fn string) *EventLogger {
	return &EventLogger{
		fn:     filepath.Join(savedir, fn),
		e_chan: make(chan string, 100), // larger buffer
	}
}

func (e *EventLogger) FileName() string {
	return e.fn
}

func (e *EventLogger) Start() {

	var err error
	e.file, err = os.OpenFile(e.fn, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		logrus.Fatalf("Failed to open log file %s: %v", e.fn, err)
	}
	logrus.Tracef("EventLogger: started writing to %s", e.fn)

	e.wg.Add(1)

	go func() {
		defer e.wg.Done()
		for event := range e.e_chan {
			event = fmt.Sprintf("%s,", event) // with " " to separate events
			_, err := e.file.WriteString(event)
			if err != nil {
				logrus.Errorf("Failed to write event: %v", err)
			}
		}
	}()
}

func (e *EventLogger) LogEvent(event string) {
	e.e_chan <- event
}

func (e *EventLogger) Stop() {
	close(e.e_chan)
	e.wg.Wait()
	if e.file != nil {
		e.file.Close()
	}
	logrus.Tracef("EventLogger: stopped and file closed")
}

// Global interface
func Initialize(filename string) {
	globalLogger = NewEventLogger(filename)
	globalLogger.Start()
}

func LogEvent(event string) {
	if globalLogger != nil {
		globalLogger.e_chan <- event
	} else {
		logrus.Warn("LogEvent called before Initialize()")
	}
}

func Shutdown() {
	if globalLogger != nil {
		globalLogger.Stop()
	}
}
