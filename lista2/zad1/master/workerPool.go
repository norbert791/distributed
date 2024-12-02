package main

import (
	"fmt"
	"io"
	"os/exec"
)

type Worker struct {
	cmd  *exec.Cmd
	pipe io.Writer
}

func NewWorker(command string) (*Worker, error) {
	worker := &Worker{}
	worker.cmd = exec.Command(command)

	var err error
	worker.pipe, err = worker.cmd.StdinPipe()
	if err != nil {
		return nil, err
	}

	return worker, nil
}

func (w *Worker) Run() error {
	err := w.cmd.Start()
	if err != nil {
		return err
	}

	return nil
}

func (w *Worker) Stop() error {
	_, err := w.pipe.Write([]byte("done"))
	if err != nil {
		return err
	}

	return w.cmd.Wait()
}

func (w *Worker) Map(inputFile, outputFile string) error {
	_, err := w.pipe.Write([]byte(fmt.Sprintf("%s %s %s", "map", inputFile, outputFile)))
	if err != nil {
		return err
	}

	return err
}

func (w *Worker) Reduce(inputFile, outputFile string) error {
	_, err := w.pipe.Write([]byte(fmt.Sprintf("%s %s %s", "reduce", inputFile, outputFile)))
	if err != nil {
		return err
	}

	return nil
}
