package fsm

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"

	"github.com/boltdb/bolt"
	"github.com/hashicorp/raft"
)

type OperationType string

const (
	OperationTypeInsert OperationType = "insert"
	OperationTypeDelete OperationType = "delete"
	OperationTypeGet    OperationType = "get"
)

type LogEntry struct {
	Key   []byte
	Value []byte
	Type  OperationType
}

type BoltFSM struct {
	db *bolt.DB
}

func NewBoltFSM(db *bolt.DB) *BoltFSM {
	return &BoltFSM{db: db}
}

func (f *BoltFSM) Apply(log *raft.Log) interface{} {
	var entry LogEntry
	if err := json.NewDecoder(bytes.NewReader(log.Data)).Decode(&entry); err != nil {
		return err
	}

	var value []byte
	err := f.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("fsm"))
		if err != nil {
			return err
		}
		switch entry.Type {
		case OperationTypeInsert:
			return bucket.Put(entry.Key, entry.Value)
		case OperationTypeDelete:
			return bucket.Delete(entry.Key)
		case OperationTypeGet:
			value = bucket.Get(entry.Key)
			return nil
		default:
			return errors.New("unknown operation")
		}
	})
	if err != nil {
		return err
	}

	// Note: unless entry.Type is Get this will return nil
	return value
}

func (f *BoltFSM) Snapshot() (raft.FSMSnapshot, error) {
	return &boltSnapshot{db: f.db}, nil
}

func (f *BoltFSM) Restore(rc io.ReadCloser) error {
	defer rc.Close()
	decoder := json.NewDecoder(rc)
	return f.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("fsm"))
		if err != nil {
			return err
		}
		for {
			var entry LogEntry
			if err := decoder.Decode(&entry); err != nil {
				if err == io.EOF {
					break
				}
				return err
			}
			switch entry.Type {
			case OperationTypeInsert:
				if err := bucket.Put(entry.Key, entry.Value); err != nil {
					return err
				}
			case OperationTypeDelete:
				if err := bucket.Delete(entry.Key); err != nil {
					return err
				}
			}
		}
		return nil
	})
}

type boltSnapshot struct {
	db *bolt.DB
}

func (s *boltSnapshot) Persist(sink raft.SnapshotSink) error {
	err := s.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("fsm"))
		if bucket == nil {
			return nil
		}
		return bucket.ForEach(func(k, v []byte) error {
			entry := LogEntry{Key: k, Value: v, Type: OperationTypeInsert}
			var buf bytes.Buffer
			if err := json.NewEncoder(&buf).Encode(entry); err != nil {
				return err
			}
			_, err := sink.Write(buf.Bytes())
			return err
		})
	})
	if err != nil {
		sink.Cancel()
		return err
	}
	sink.Close()
	return nil
}

func (s *boltSnapshot) Release() {}
