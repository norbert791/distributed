package main

import (
	"errors"
	"fmt"
	"log"
	"norbert791/distributed/lista3/zad1/fsm"
	store "norbert791/distributed/lista3/zad1/logstore"
	"norbert791/distributed/lista3/zad1/stablestore"
	"os"
	"path/filepath"
	"time"

	"github.com/boltdb/bolt"
	"github.com/hashicorp/raft"
)

type Node struct {
	db            *bolt.DB
	transport     *raft.InmemTransport
	snapshotStore raft.SnapshotStore
	logStore      raft.LogStore
	stableStore   raft.StableStore
	fsm           raft.FSM
	config        *raft.Config
	raftNode      *raft.Raft
}

func (n *Node) Close() error {
	var err error
	err = errors.Join(err, n.raftNode.Shutdown().Error())
	err = errors.Join(err, n.db.Close())
	return err
}

type NodePool struct {
	nodes []*Node
}

func NewNodePool(numOfNodes int) (*NodePool, error) {
	nodes := make([]*Node, numOfNodes)
	for i := range numOfNodes {
		config := raft.DefaultConfig()
		config.LocalID = raft.ServerID("node" + string(i))
		config.HeartbeatTimeout = 1000 * time.Millisecond
		config.ElectionTimeout = 1000 * time.Millisecond
		config.CommitTimeout = 500 * time.Millisecond

		// Create an in-memory transport
		_, transport := raft.NewInmemTransport("")
		if transport == nil {
			return nil, errors.New("failed to create transport")
		}

		// Create a snapshot store
		snapshotStore, err := raft.NewFileSnapshotStore(filepath.Join(os.TempDir(), "raft-snapshots"), 1, os.Stderr)
		if err != nil {
			log.Fatalf("failed to create snapshot store: %v", err)
		}

		boltdb, err := bolt.Open(filepath.Join(os.TempDir(), fmt.Sprintf("raft-log-%d.bolt", i)), 0600, nil)
		if err != nil {
			return nil, err
		}

		// Create a log store and stable store
		logStore, err := store.NewBoltStore(boltdb)
		if err != nil {
			log.Fatalf("failed to create log store: %v", err)
		}
		stableStore := stablestore.NewBoltStableStore(boltdb)
		if err != nil {
			return nil, err
		}
		fsm := fsm.NewBoltFSM(boltdb)

		nodes[i] = &Node{
			db:            boltdb,
			transport:     transport,
			snapshotStore: snapshotStore,
			logStore:      logStore,
			stableStore:   stableStore,
			fsm:           fsm,
			config:        config,
		}
	}

	for i, node := range nodes {
		for j, otherNode := range nodes {
			if i == j {
				continue
			}
			node.transport.Connect(otherNode.transport.LocalAddr(), otherNode.transport)
			otherNode.transport.Connect(node.transport.LocalAddr(), node.transport)
		}
	}

	for _, node := range nodes {
		raftNode, err := raft.NewRaft(node.config, node.fsm, node.logStore, node.stableStore, node.snapshotStore, node.transport)
		if err != nil {
			return nil, err
		}
		node.raftNode = raftNode
	}

	return &NodePool{nodes: nodes}, nil
}

func (np *NodePool) BootstrapCluster() error {
	if len(np.nodes) == 0 {
		return errors.New("no nodes in the pool")
	}

	root := np.nodes[0]
	err := root.raftNode.BootstrapCluster(
		raft.Configuration{Servers: []raft.Server{{ID: root.config.LocalID, Address: root.transport.LocalAddr()}}}).Error()
	if err != nil {
		return err
	}
	for _, node := range np.nodes[1:] {
		err := root.raftNode.AddVoter(node.config.LocalID, node.transport.LocalAddr(), 0, 10*time.Second).Error()
		if err != nil {
			return err
		}
	}

	return nil
}

func (np *NodePool) Close() error {
	var err error
	for _, node := range np.nodes {
		err = errors.Join(err, node.Close())
	}
	return err
}
func (np *NodePool) Leader() *Node {
	for _, node := range np.nodes {
		if node.raftNode.State() == raft.Leader {
			return node
		}
	}
	return nil
}
