package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

func performMapping(inputPath, outputPath string) error {
	outputFile, err := os.Create(outputPath)
	if err != nil {
		return err
	}
	defer outputFile.Close()

	inputFile, err := os.Open(inputPath)
	if err != nil {
		return err
	}
	defer inputFile.Close()

	words := make(map[string]uint64)
	scanner := bufio.NewScanner(inputFile)
	scanner.Split(bufio.ScanWords)

	for scanner.Scan() {
		w := scanner.Text()
		words[w]++
	}

	for w, v := range words {
		output := fmt.Sprintf("%s %d\n", w, v)
		if _, err := outputFile.Write([]byte(output)); err != nil {
			return err
		}
	}

	return nil
}

func performReduce(inputPath, outputPath string) error {
	outputFile, err := os.Create(outputPath)
	if err != nil {
		return err
	}
	defer outputFile.Close()

	inputFile, err := os.Open(inputPath)
	if err != nil {
		return err
	}
	defer inputFile.Close()

	words := make(map[string]uint64)
	scanner := bufio.NewScanner(inputFile)

	for scanner.Scan() {
		w := scanner.Text()
		var (
			key string
			val uint64
		)
		n, err := fmt.Sscanf(w, "%s %d\n", &key, &val)
		if err != nil {
			return err
		}
		if n != 2 {
			return errors.New("invalid num of parsed params")
		}
		words[key] += val
	}

	for w, v := range words {
		output := fmt.Sprintf("%s %d\n", w, v)
		if _, err := outputFile.Write([]byte(output)); err != nil {
			return err
		}
	}

	return nil
}

func main() {
	defer func() {
		rec := recover()
		if v, ok := rec.(error); ok {
			fmt.Printf("error: %s\n", v)
		}
	}()

	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := scanner.Text()
		splitted := strings.Split(text, " ")

		command := splitted[0]

		var err error
		switch command {
		case "map":
			input := splitted[1]
			otput := splitted[2]
			err = performMapping(input, otput)
		case "reduce":
			input := splitted[1]
			otput := splitted[2]
			err = performReduce(input, otput)
		case "done":
			return
		default:
			fmt.Println("error: unknown command")
		}

		if err != nil {
			fmt.Printf("error: %s\n", err)
		} else {
			fmt.Println("ok")
		}
	}
}
