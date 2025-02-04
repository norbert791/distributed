package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"io"
	"log"
	"net/http"

	"norbert791/distributed/lista3/zad1/fsm"
)

func main() {
	// Parse command line arguments
	key := flag.String("key", "", "Key to store")
	value := flag.String("value", "", "Value to store")
	opType := flag.String("type", "insert", "Operation type (insert, delete, get)")
	flag.Parse()

	if *key == "" {
		log.Fatal("Key is required")
	}

	// Create a log entry
	entry := fsm.LogEntry{
		Key:   []byte(*key),
		Value: []byte(*value),
		Type:  fsm.OperationType(*opType),
	}

	// Marshal the log entry
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(entry); err != nil {
		log.Fatalf("failed to encode log entry: %v", err)
	}

	resp, err := http.DefaultClient.Post("http://localhost:8080/db", "application/json", &buf)
	if err != nil {
		log.Fatalf("failed to send request: %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		bts, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Fatalf("unexpected status code: %d, failed to read body: %v", resp.StatusCode, err)
		}
		log.Fatalf("unexpected status code: %d, body: %v", resp.StatusCode, string(bts))
	} else {
		log.Println("Request successful")
	}
}
