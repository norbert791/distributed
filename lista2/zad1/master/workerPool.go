package main

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os/exec"
	"strings"
)

type Worker struct {
	cmd     *exec.Cmd
	inPipe  io.Writer
	outPipe io.Reader
}

func NewWorker(command string) (*Worker, error) {
	worker := &Worker{}
	worker.cmd = exec.Command(command)

	var err error
	worker.inPipe, err = worker.cmd.StdinPipe()
	if err != nil {
		return nil, err
	}

	worker.outPipe, err = worker.cmd.StdoutPipe()
	if err != nil {
		return nil, err
	}

	return worker, nil
}

// TODO: use context?

func (w *Worker) Run() error {
	err := w.cmd.Start()
	if err != nil {
		return err
	}

	return nil
}

func (w *Worker) Close() error {
	_, err := w.inPipe.Write([]byte("done\n"))
	if err != nil {
		return err
	}

	_, err = io.ReadAll(w.outPipe)
	if err != nil {
		return err
	}

	return w.cmd.Wait()
}

func (w *Worker) Wait() error {
	scanner := bufio.NewScanner(w.outPipe)

	for scanner.Scan() {
		message := scanner.Text()

		if strings.HasPrefix(message, "ok") {
			return nil
		} else if strings.HasPrefix(message, "error") {
			return errors.New(message)
		} else {
			fmt.Println(message)
		}

	}

	return nil
}

func (w *Worker) Map(inputFile, outputFile string) error {
	_, err := w.inPipe.Write([]byte(fmt.Sprintf("map %s %s\n", inputFile, outputFile)))
	if err != nil {
		return err
	}

	return nil
}

func (w *Worker) Reduce(inputFile, outputFile string) error {
	_, err := w.inPipe.Write([]byte(fmt.Sprintf("reduce %s %s\n", inputFile, outputFile)))
	if err != nil {
		return err
	}

	return nil
}

type WorkerPool struct {
	workers     []*Worker
	interrupted bool
}

func WorkerPoolOpen(num uint, command string) (*WorkerPool, error) {
	result := &WorkerPool{}
	result.workers = make([]*Worker, num)

	for i := range num {
		var err error
		result.workers[i], err = NewWorker(command)
		if err != nil {
			for j := range i {
				err := result.workers[j].Close()
				if err != nil {
					panic("stop error triggered after first error")
				}
			}
			return nil, err
		}

		err = result.workers[i].Run()
		if err != nil {
			return nil, err
		}
	}

	return result, nil
}

func (w *WorkerPool) Map(inputFiles, outputFiles []string) error {
	if len(inputFiles) != len(outputFiles) {
		return errors.New("len of inputFiels must equal len of outputFiles")
	}

	w.interrupted = true

	for i, worker := range w.workers {
		err := worker.Map(inputFiles[i], outputFiles[i])
		if err != nil {
			return err
		}
	}

	for _, worker := range w.workers {
		err := worker.Wait()
		if err != nil {
			return err
		}
	}

	w.interrupted = false
	return nil
}

func (w *WorkerPool) Reduce(inputFiles, outputFiles []string) error {
	if len(inputFiles) != len(outputFiles) {
		return errors.New("len of inputFiels must equal len of outputFiles")
	}

	w.interrupted = true

	for i, worker := range w.workers {
		err := worker.Reduce(inputFiles[i], outputFiles[i])
		if err != nil {
			return err
		}
	}

	for _, worker := range w.workers {
		err := worker.Wait()
		if err != nil {
			return err
		}
	}

	w.interrupted = false

	return nil
}

func (w *WorkerPool) Close() error {
	if w.interrupted {
		return w.kill()
	}

	var err error
	for _, worker := range w.workers {
		err = errors.Join(worker.Close())
	}

	if err != nil {
		return errors.Join(err, w.kill())
	}

	return nil
}

func (w *WorkerPool) kill() error {
	var err error
	for _, worker := range w.workers {
		if worker == nil {
			continue
		}
		err = errors.Join(err, worker.cmd.Process.Kill())
	}

	return err
}
