package ds

import (
	"bufio"
	"fmt"
	"io"
)

type twoNode struct {
	data string
	next *twoNode
	prev *twoNode
}

type Linked struct {
	head *twoNode
	tail *twoNode
	size int
}

func (l *Linked) findNode(value string) *twoNode {
	node := l.head
	for node != nil && node.data != value {
		node = node.next
	}
	return node
}

func (l *Linked) unlink(node *twoNode) string {
	if node.prev != nil {
		node.prev.next = node.next
	} else {
		l.head = node.next
	}
	if node.next != nil {
		node.next.prev = node.prev
	} else {
		l.tail = node.prev
	}
	l.size--
	return node.data
}

func (l *Linked) Find(value string) int {
	i := 0
	for node := l.head; node != nil; node = node.next {
		if node.data == value {
			return i
		}
		i++
	}
	return -1
}

func (l *Linked) PushBack(v string) {
	node := &twoNode{data: v, prev: l.tail}
	if l.tail != nil {
		l.tail.next = node
	} else {
		l.head = node
	}
	l.tail = node
	l.size++
}

func (l *Linked) PushFront(v string) {
	node := &twoNode{data: v, next: l.head}
	if l.head != nil {
		l.head.prev = node
	} else {
		l.tail = node
	}
	l.head = node
	l.size++
}

func (l *Linked) PushAfter(prev, v string) error {
	node := l.findNode(prev)
	if node == nil {
		return fmt.Errorf("%w: %q", ErrNotFound, prev)
	}
	if node == l.tail {
		l.PushBack(v)
		return nil
	}
	fresh := &twoNode{data: v, next: node.next, prev: node}
	node.next.prev = fresh
	node.next = fresh
	l.size++
	return nil
}

func (l *Linked) PushBefore(next, v string) error {
	node := l.findNode(next)
	if node == nil {
		return fmt.Errorf("%w: %q", ErrNotFound, next)
	}
	if node == l.head {
		l.PushFront(v)
		return nil
	}
	fresh := &twoNode{data: v, next: node, prev: node.prev}
	node.prev.next = fresh
	node.prev = fresh
	l.size++
	return nil
}

func (l *Linked) PopFront() (string, error) {
	if l.head == nil {
		return "", ErrEmpty
	}
	return l.unlink(l.head), nil
}

func (l *Linked) PopBack() (string, error) {
	if l.tail == nil {
		return "", ErrEmpty
	}
	return l.unlink(l.tail), nil
}

func (l *Linked) PopAfter(prev string) (string, error) {
	node := l.findNode(prev)
	if node == nil {
		return "", fmt.Errorf("%w: %q", ErrNotFound, prev)
	}
	if node.next == nil {
		return "", ErrNoNeighbor
	}
	return l.unlink(node.next), nil
}

func (l *Linked) PopBefore(next string) (string, error) {
	node := l.findNode(next)
	if node == nil {
		return "", fmt.Errorf("%w: %q", ErrNotFound, next)
	}
	if node.prev == nil {
		return "", ErrNoNeighbor
	}
	return l.unlink(node.prev), nil
}

func (l *Linked) DelByValue(value string) bool {
	node := l.findNode(value)
	if node == nil {
		return false
	}
	l.unlink(node)
	return true
}

func (l *Linked) Clear() { *l = Linked{} }

func (l *Linked) Len() int { return l.size }

func (l *Linked) Empty() bool { return l.size == 0 }

func (l *Linked) Items() []string {
	out := make([]string, 0, l.size)
	for node := l.head; node != nil; node = node.next {
		out = append(out, node.data)
	}
	return out
}

func (l *Linked) String() string { return joinWithSpaces(l.Items()) }

func (l *Linked) StringReverse() string {
	out := make([]string, 0, l.size)
	for node := l.tail; node != nil; node = node.prev {
		out = append(out, node.data)
	}
	return joinWithSpaces(out)
}

func (l *Linked) Clone() *Linked {
	c := &Linked{}
	for node := l.head; node != nil; node = node.next {
		c.PushBack(node.data)
	}
	return c
}

func (l *Linked) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, "LIN2", l.Items())
}

func (l *Linked) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, "LIN2")
	if err != nil {
		return err
	}
	l.replace(items)
	return nil
}

func (l *Linked) SerializeText(w io.Writer) error {
	return serializeStringsText(w, "LinkedList", l.Items())
}

func (l *Linked) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, "LinkedList")
	if err != nil {
		return err
	}
	l.replace(items)
	return nil
}

func (l *Linked) replace(items []string) {
	l.Clear()
	for _, s := range items {
		l.PushBack(s)
	}
}
