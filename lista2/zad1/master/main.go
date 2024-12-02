package main

import (
	"bufio"
	"errors"
	"fmt"
	"hash/fnv"
	"io"
	"io/fs"
	"os"
	"path"
	"strconv"
	"strings"
)

func main() {
	args := os.Args
	if len(args) != 5 {
		fmt.Println("invalid number of args")
		return
	}

	binPath := args[1]
	workers, err := strconv.ParseUint(args[2], 10, 64)
	if err != nil {
		fmt.Println(err)
		return
	}

	filepath := args[3]
	outpath := args[4]

	workdir, err := os.MkdirTemp("", "mapReduce")
	if err != nil {
		if !errors.Is(err, fs.ErrExist) {
			fmt.Println(err)
			return
		}
	}
	defer os.RemoveAll(workdir)

	mapInputs := make([]string, 0, workers)
	mapOutputs := make([]string, 0, workers)
	reduceInputs := make([]string, 0, workers)
	reduceOutputs := make([]string, 0, workers)
	for i := range workers {
		mapInputs = append(mapInputs, path.Join(workdir, fmt.Sprintf("map-input-%d", i)))
		mapOutputs = append(mapOutputs, path.Join(workdir, fmt.Sprintf("map-output-%d", i)))
		reduceInputs = append(reduceInputs, path.Join(workdir, fmt.Sprintf("reduce-input-%d", i)))
		reduceOutputs = append(reduceOutputs, path.Join(workdir, fmt.Sprintf("reduce-output-%d", i)))
	}
	err = ShardInput(mapInputs, filepath)
	if err != nil {
		fmt.Println("sharding failed", err)
		return
	}

	fmt.Println("sharding complete")

	// Setup workers
	pool, err := WorkerPoolOpen(uint(workers), binPath)
	if err != nil {
		fmt.Println("worker setup failed", err)
		return
	}

	defer func() {
		if err := pool.Close(); err != nil {
			panic(err)
		}
	}()

	fmt.Println("workers started")

	// Run mappers
	err = pool.Map(mapInputs, mapOutputs)
	if err != nil {
		fmt.Println("map failed", err)
		return
	}

	fmt.Println("Mappers finished")

	err = ShuffleMapped(mapOutputs, reduceInputs)
	if err != nil {
		fmt.Println("shuffle failed", err)
		return
	}

	fmt.Println("shuffle finished")

	// Run reducers
	err = pool.Reduce(reduceInputs, reduceOutputs)
	if err != nil {
		fmt.Println("reduce failed")
		return
	}

	fmt.Println("Reducers finished")

	outputFile, err := os.Create(outpath)
	if err != nil {
		fmt.Println("combine file create failed", err)
		return
	}
	defer outputFile.Close()

	err = CombineResults(reduceOutputs, outputFile)
	if err != nil {
		fmt.Println("combine failed", err)
		return
	}

	fmt.Println("Combined")
}

func CombineResults(filenames []string, writer io.Writer) error {
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

func ShardInput(filenames []string, input string) error {
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

func ShuffleMapped(input []string, output []string) error {
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
