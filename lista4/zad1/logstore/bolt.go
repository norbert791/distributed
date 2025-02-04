package logstore

import (
	"bytes"
	"encoding/binary"
	"encoding/json"

	"github.com/boltdb/bolt"
	"github.com/hashicorp/raft"
)

type BoltStore struct {
	db *bolt.DB
}

func NewBoltStore(db *bolt.DB) (*BoltStore, error) {
	return &BoltStore{db: db}, nil
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

func itob(v uint64) []byte {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, v)
	return b
}

func marshalLog(log *raft.Log) ([]byte, error) {
	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	if err := enc.Encode(log); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func unmarshalLog(data []byte, log *raft.Log) error {
	buf := bytes.NewBuffer(data)
	dec := json.NewDecoder(buf)
	return dec.Decode(log)
}
