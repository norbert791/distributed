package store

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"io"

	"github.com/boltdb/bolt"
	"github.com/hashicorp/raft"
)

type BoltStore struct {
	db       *bolt.DB
	filePath string
}

func NewBoltStore(filePath string) (*BoltStore, error) {
	db, err := bolt.Open(filePath, 0600, nil)
	if err != nil {
		return nil, err
	}
	return &BoltStore{db: db, filePath: filePath}, nil
}

func (b *BoltStore) FirstIndex() (uint64, error) {
	var firstIndex uint64
	err := b.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("logs"))
		if bucket == nil {
			return nil
		}
		cursor := bucket.Cursor()
		k, _ := cursor.First()
		if k != nil {
			firstIndex = binary.BigEndian.Uint64(k)
		}
		return nil
	})
	return firstIndex, err
}

func (b *BoltStore) LastIndex() (uint64, error) {
	var lastIndex uint64
	err := b.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("logs"))
		if bucket == nil {
			return nil
		}
		cursor := bucket.Cursor()
		k, _ := cursor.Last()
		if k != nil {
			lastIndex = binary.BigEndian.Uint64(k)
		}
		return nil
	})
	return lastIndex, err
}

func (b *BoltStore) GetLog(index uint64, log *raft.Log) error {
	return b.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("logs"))
		if bucket == nil {
			return nil
		}
		v := bucket.Get(itob(index))
		if v == nil {
			return nil
		}
		return unmarshalLog(v, log)
	})
}

func (b *BoltStore) StoreLog(log *raft.Log) error {
	return b.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("logs"))
		if err != nil {
			return err
		}
		data, err := marshalLog(log)
		if err != nil {
			return err
		}
		return bucket.Put(itob(log.Index), data)
	})
}

func (b *BoltStore) StoreLogs(logs []*raft.Log) error {
	return b.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("logs"))
		if err != nil {
			return err
		}
		for _, log := range logs {
			data, err := marshalLog(log)
			if err != nil {
				return err
			}
			if err := bucket.Put(itob(log.Index), data); err != nil {
				return err
			}
		}
		return nil
	})
}

func (b *BoltStore) DeleteRange(min, max uint64) error {
	return b.db.Update(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("logs"))
		if bucket == nil {
			return nil
		}
		cursor := bucket.Cursor()
		for k, _ := cursor.Seek(itob(min)); k != nil && binary.BigEndian.Uint64(k) <= max; k, _ = cursor.Next() {
			if err := bucket.Delete(k); err != nil {
				return err
			}
		}
		return nil
	})
}

func (b *BoltStore) Set(key []byte, val []byte) error {
	return b.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("stable"))
		if err != nil {
			return err
		}
		return bucket.Put(key, val)
	})
}

func (b *BoltStore) Get(key []byte) ([]byte, error) {
	var val []byte
	err := b.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("stable"))
		if bucket == nil {
			return nil
		}
		val = bucket.Get(key)
		return nil
	})
	return val, err
}

func (b *BoltStore) SetUint64(key []byte, val uint64) error {
	return b.Set(key, itob(val))
}

func (b *BoltStore) GetUint64(key []byte) (uint64, error) {
	val, err := b.Get(key)
	if err != nil {
		return 0, err
	}
	if val == nil {
		return 0, nil
	}
	return binary.BigEndian.Uint64(val), nil
}

func (b *BoltStore) Apply(log *raft.Log) interface{} {
	// Apply the log entry to the BoltStore
	err := b.StoreLog(log)
	if err != nil {
		return err
	}
	return nil
}

func (b *BoltStore) Snapshot() (raft.FSMSnapshot, error) {
	// Return a snapshot of the current state
	return &boltSnapshot{store: b}, nil
}

func (b *BoltStore) Restore(rc io.ReadCloser) error {
	// Restore the state from a snapshot
	defer rc.Close()
	decoder := gob.NewDecoder(rc)
	for {
		var log raft.Log
		if err := decoder.Decode(&log); err != nil {
			if err == io.EOF {
				break
			}
			return err
		}
		if err := b.StoreLog(&log); err != nil {
			return err
		}
	}
	return nil
}

type boltSnapshot struct {
	store *BoltStore
}

func (s *boltSnapshot) Persist(sink raft.SnapshotSink) error {
	// Persist the snapshot to the sink
	err := s.store.db.View(func(tx *bolt.Tx) error {
		bucket := tx.Bucket([]byte("logs"))
		if bucket == nil {
			return nil
		}
		return bucket.ForEach(func(k, v []byte) error {
			_, err := sink.Write(v)
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

func (s *boltSnapshot) Release() {
	// Release the snapshot
}

func itob(v uint64) []byte {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, v)
	return b
}

func marshalLog(log *raft.Log) ([]byte, error) {
	var buf bytes.Buffer
	enc := gob.NewEncoder(&buf)
	if err := enc.Encode(log); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func unmarshalLog(data []byte, log *raft.Log) error {
	buf := bytes.NewBuffer(data)
	dec := gob.NewDecoder(buf)
	return dec.Decode(log)
}
