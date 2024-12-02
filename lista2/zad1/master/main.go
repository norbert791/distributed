package main

import (
	"bufio"
	"fmt"
	"os"
	"path"
	"strconv"
)

func combineMappings()

func shardInput(filenames []string, input string) error {
	inputFile, err := os.Open(input)
	if err != nil {
		return err
	}
	defer inputFile.Close()

	files := make([]*os.File, len(filenames))
	for i, name := range filenames {
		var err error
		files[i], err = os.Create(name)
		if err != nil {
			return err
		} else {
			// Note: This will trigger when shardInput leaves. This is an intented behaviour.
			defer files[i].Close()
		}
	}

	scanner := bufio.NewScanner(inputFile)

	counter := 0
	for scanner.Scan() {
		line := scanner.Text()
		files[counter].WriteString(line)
		files[counter].WriteString("\n")
		counter++
		counter %= len(files)
	}

	return nil
}

func main() {
	args := os.Args
	if len(args) != 3 {
		fmt.Println("invalid number of args")
		return
	}

	filepath := args[1]
	workers, err := strconv.ParseUint(args[2], 10, 64)
	if err != nil {
		fmt.Println(err)
		return
	}

	workdir := ".mapReduce"
	err = os.Mkdir(workdir, os.ModeDir)
	if err != nil {
		fmt.Println(err)
		return
	}

	mapInputs := make([]string, 0, workers)
	for i := range workers {
		fname := path.Join(workdir, fmt.Sprintf("map-input-%d", i))
		mapInputs = append(mapInputs, fname)
	}
	shardInput(mapInputs, filepath)
}
