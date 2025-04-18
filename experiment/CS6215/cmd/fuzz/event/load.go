package event

import (
	"os"
)

func LoadTrace(fn string) (trace string, err error) {
	file, err := os.OpenFile(fn, os.O_RDONLY, 0644)
	if err != nil {
		return
	}

	bytes := make([]byte, 1024)
	for {
		n, err := file.Read(bytes)
		if err != nil {
			break
		}
		trace += string(bytes[:n])
	}

	return
}
