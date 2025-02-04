package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"norbert791/distributed/lista3/zad1/fsm"
	"os"
	"os/signal"
	"sync"
	"time"
)

func main() {
	ctx, cancel := signal.NotifyContext(context.Background(), os.Interrupt, os.Kill)
	defer cancel()

	// Create a new NodePool
	pool, err := NewNodePool(3)
	if err != nil {
		log.Fatalf("failed to create node pool: %v", err)
	}
	defer pool.Close()
	if err := pool.BootstrapCluster(); err != nil {
		log.Fatalf("failed to bootstrap cluster: %v", err)
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/db", func(w http.ResponseWriter, r *http.Request) {
		log.Println("Received request")

		bts := make([]byte, r.ContentLength)
		if _, err := r.Body.Read(bts); err != nil {
			if !errors.Is(err, io.EOF) {
				http.Error(w, fmt.Sprintf("failed to read request body %s", err), http.StatusBadRequest)
				return
			}
		}

		var req fsm.LogEntry
		// assert schema
		decoder := json.NewDecoder(bytes.NewBuffer(bts))
		decoder.DisallowUnknownFields()
		if err := decoder.Decode(&req); err != nil {
			http.Error(w, "failed to decode JSON body", http.StatusBadRequest)
			return
		}

		leader := pool.Leader()
		if leader == nil {
			http.Error(w, "no leader available", http.StatusInternalServerError)
			return
		}

		future := leader.raftNode.Apply(bts, 5*time.Second)
		if err := future.Error(); err != nil {
			http.Error(w, "failed to apply log entry", http.StatusInternalServerError)
			return
		}

		if req.Type != fsm.OperationTypeGet {
			w.WriteHeader(http.StatusOK)
			return
		}

		res := future.Response()
		resVal, ok := res.([]byte)
		if !ok {
			http.Error(w, "failed to get value", http.StatusInternalServerError)
			return
		}

		w.Write(resVal)

	})

	server := &http.Server{
		Addr:    "localhost:8080",
		Handler: mux,
	}

	var wg sync.WaitGroup

	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := server.ListenAndServe(); err != nil {
			if errors.Is(err, http.ErrServerClosed) {
				return
			}
			log.Fatalf("failed to start HTTP server: %v", err)
		}
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		<-ctx.Done()
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := server.Shutdown(ctx); err != nil {
			log.Fatalf("failed to shutdown HTTP server: %v", err)
		}
	}()

	log.Println("Server started")
	wg.Wait()
}
