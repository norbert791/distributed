package main

import (
	"encoding/binary"
	"time"
)

type UUID [12]byte

type Generator struct {
	Counter   uint16
	ThreadID  uint16
	ClusterID uint16
	PodID     uint16
}

func (g *Generator) Generate() UUID {
	g.Counter++
	var result UUID
	ts := time.Now()
	epoch := ts.Unix() / 1000000
	truncatedEpoch := uint32(epoch) & 0x0000FFFF
	binary.LittleEndian.PutUint32(result[:], truncatedEpoch)
	binary.LittleEndian.PutUint16(result[4:], g.Counter)
	binary.LittleEndian.PutUint16(result[6:], g.ThreadID)
	binary.LittleEndian.PutUint16(result[8:], g.ClusterID)
	binary.LittleEndian.PutUint16(result[10:], g.PodID)

	return result
}
