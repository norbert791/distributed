package main

import (
	"os"
	"strconv"
	"sync"
	"time"
)

const numOfThreads = 20

func main() {

	clusterID := os.Getenv("ClusterID")
	nodeID := os.Getenv("NodeID")

	if clusterID == "" || nodeID == "" {
		clusterID = "1"
		nodeID = "1"
		// XXX: normally we would raise error
	}

	clusterIDInt, err := strconv.ParseUint(clusterID, 10, 16)
	if err != nil {
		panic(err)
	}

	nodeIDInt, err := strconv.ParseUint(nodeID, 10, 16)
	if err != nil {
		panic(err)
	}

	var generators [numOfThreads]Generator
	for i := range generators {
		generators[i] = Generator{
			Counter:   0,
			ThreadID:  uint16(i + 1),
			ClusterID: uint16(clusterIDInt),
			PodID:     uint16(nodeIDInt),
		}
	}

	start := time.Now()

	wg := sync.WaitGroup{}
	var counters [numOfThreads]uint64
	for i := range numOfThreads {
		wg.Add(1)
		go func() {
			defer wg.Done()
			fin := time.After(10 * time.Second)
			for {
				select {
				case <-fin:
					return
				default:
					for range 200 {
						_ = generators[i].Generate()
						counters[i]++
					}
				}
			}
		}()
	}

	wg.Wait()
	finTime := time.Now()

	deltaTime := finTime.Sub(start)

	var finalResult uint64
	for _, val := range counters {
		finalResult += val
	}

	idsPerSecond := float64(finalResult) / deltaTime.Seconds()

	println("Generated", idsPerSecond, "IDs per second")
}
