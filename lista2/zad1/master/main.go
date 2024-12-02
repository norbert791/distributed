package main

import (
	"bufio"
	"errors"
	"fmt"
	"hash/fnv"
	"io"
	"os"
	"path"
	"strconv"
	"strings"
)

func combineMappings(filenames []string, writer io.Writer) error {
	for _, name := range filenames {
		err := func() error {
			f, err := os.Open(name)
			if err != nil {
				return err
			}
			defer f.Close()

			scanner := bufio.NewScanner(f)
			for scanner.Scan() {
				line := scanner.Text()
				_, err := writer.Write([]byte(line + "\n"))
				if err != nil {
					return err
				}
			}

			return nil
		}()
		if err != nil {
			return err
		}
	}

	return nil
}

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

func shuffleMapped(input []string, output []string) error {
	outputFiles := make([]*os.File, len(input))
	for i, name := range output {
		var err error
		outputFiles[i], err = os.Create(name)
		if err != nil {
			return err
		} else {
			// Note: This will trigger when shardInput leaves. This is an intented behaviour.
			defer outputFiles[i].Close()
		}
	}

	for _, name := range input {
		err := func() error {
			input, err := os.Open(name)
			if err != nil {
				return err
			}
			defer input.Close()

			scanner := bufio.NewScanner(input)

			hashFun := fnv.New32()
			for scanner.Scan() {
				line := scanner.Text()
				splitted := strings.Split(line, " ")
				if len(splitted) != 2 {
					return errors.New("malformed pairs")
				}
				key := splitted[0]
				val := splitted[1]
				hashFun.Reset()
				keyHash, err := hashFun.Write([]byte(key))
				if err != nil {
					return err
				}
				keyHash %= len(outputFiles)
				_, err = outputFiles[keyHash].WriteString(fmt.Sprintf("%s %s\n", key, val))
				if err != nil {
					return err
				}
			}
			return nil
		}()
		if err != nil {
			return err
		}
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
	mapOutputs := make([]string, 0, workers)
	reduceInputs := make([]string, 0, workers)
	reduceOutputs := make([]string, 0, workers)
	for i := range workers {
		mapInputs = append(mapInputs, path.Join(workdir, fmt.Sprintf("map-input-%d", i)))
		mapOutputs = append(mapOutputs, path.Join(workdir, fmt.Sprintf("map-output-%d", i)))
	}
	err = shardInput(mapInputs, filepath)
	if err != nil {
		fmt.Println("sharding failed")
		return
	}
	// Setup workers

	// Run mappers

	err = shuffleMapped(mapOutputs, reduceInputs)
	if err != nil {
		fmt.Println("shuffle failed")
		return
	}
	// Run reducers

	err = combineMappings(reduceOutputs, os.Stdout)
	if err != nil {
		fmt.Println("combine failed")
		return
	}

	// Close reducers
}
