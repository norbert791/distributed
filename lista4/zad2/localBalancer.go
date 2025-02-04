package main

type Balancer struct {
	counter int
	Nodes   []Generator
}

func (b *Balancer) Request() UUID {
	b.counter = b.counter % len(b.Nodes)
	node := b.Nodes[b.counter]
	return node.Generate()
}
