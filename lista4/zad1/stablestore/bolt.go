package stablestore

import (
	"encoding/binary"

	"github.com/boltdb/bolt"
)

type BoltStableStore struct {
	db *bolt.DB
}

func NewBoltStableStore(db *bolt.DB) *BoltStableStore {
	return &BoltStableStore{db: db}
}

func (b *BoltStableStore) Set(key []byte, val []byte) error {
	return b.db.Update(func(tx *bolt.Tx) error {
		bucket, err := tx.CreateBucketIfNotExists([]byte("stable"))
		if err != nil {
			return err
		}
		return bucket.Put(key, val)
	})
}

func (b *BoltStableStore) Get(key []byte) ([]byte, error) {
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

func (b *BoltStableStore) SetUint64(key []byte, val uint64) error {
	return b.Set(key, itob(val))
}

func (b *BoltStableStore) GetUint64(key []byte) (uint64, error) {
	val, err := b.Get(key)
	if err != nil {
		return 0, err
	}
	if val == nil {
		return 0, nil
	}
	return binary.BigEndian.Uint64(val), nil
}

func itob(v uint64) []byte {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, v)
	return b
}
