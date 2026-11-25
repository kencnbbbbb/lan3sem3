package ds

import (
	"bufio"
	"fmt"
	"io"
	"strings"
)

type oneNode struct {
	data string
	next *oneNode
}

type OneLinked struct {
	head *oneNode
	tail *oneNode
	size int
}

func (l *OneLinked) findPrev(value string) (*oneNode, bool) {
	var prev *oneNode
	for node := l.head; node != nil; prev, node = node, node.next {
		if node.data == value {
			return prev, true
		}
	}
	return nil, false
}

func (l *OneLinked) removeAfter(prev *oneNode) string {
	del := l.head
	if prev != nil {
		del = prev.next
		prev.next = del.next
	} else {
		l.head = del.next
	}
	if del == l.tail {
		l.tail = prev
	}
	l.size--
	return del.data
}

func (l *OneLinked) Find(value string) int {
	i := 0
	for node := l.head; node != nil; node = node.next {
		if node.data == value {
			return i
		}
		i++
	}
	return -1
}

func (l *OneLinked) PushBack(v string) {
	node := &oneNode{data: v}
	if l.tail != nil {
		l.tail.next = node
	} else {
		l.head = node
	}
	l.tail = node
	l.size++
}

func (l *OneLinked) PushFront(v string) {
	l.head = &oneNode{data: v, next: l.head}
	if l.tail == nil {
		l.tail = l.head
	}
	l.size++
}

func (l *OneLinked) PushAfter(prev, v string) error {
	prevNode, found := l.findPrev(prev)
	if !found {
		return fmt.Errorf("%w: %q", ErrNotFound, prev)
	}
	node := l.head
	if prevNode != nil {
		node = prevNode.next
	}
	node.next = &oneNode{data: v, next: node.next}
	if node == l.tail {
		l.tail = node.next
	}
	l.size++
	return nil
}

func (l *OneLinked) PushBefore(next, v string) error {
	prev, found := l.findPrev(next)
	if !found {
		return fmt.Errorf("%w: %q", ErrNotFound, next)
	}
	if prev == nil {
		l.PushFront(v)
		return nil
	}
	prev.next = &oneNode{data: v, next: prev.next}
	l.size++
	return nil
}

func (l *OneLinked) PopFront() (string, error) {
	if l.head == nil {
		return "", ErrEmpty
	}
	return l.removeAfter(nil), nil
}

func (l *OneLinked) PopBack() (string, error) {
	if l.head == nil {
		return "", ErrEmpty
	}
	var prev *oneNode
	for node := l.head; node != l.tail; node = node.next {
		prev = node
	}
	return l.removeAfter(prev), nil
}

func (l *OneLinked) PopAfter(prev string) (string, error) {
	p, found := l.findPrev(prev)
	if !found {
		return "", fmt.Errorf("%w: %q", ErrNotFound, prev)
	}
	node := l.head
	if p != nil {
		node = p.next
	}
	if node.next == nil {
		return "", ErrNoNeighbor
	}
	return l.removeAfter(node), nil
}

func (l *OneLinked) PopBefore(next string) (string, error) {
	var beforePrev, prev *oneNode
	for node := l.head; node != nil; node = node.next {
		if node.data == next {
			if prev == nil {
				return "", ErrNoNeighbor
			}
			return l.removeAfter(beforePrev), nil
		}
		beforePrev, prev = prev, node
	}
	return "", fmt.Errorf("%w: %q", ErrNotFound, next)
}

func (l *OneLinked) DelByValue(value string) bool {
	prev, found := l.findPrev(value)
	if !found {
		return false
	}
	l.removeAfter(prev)
	return true
}

func (l *OneLinked) Clear() { *l = OneLinked{} }

func (l *OneLinked) Len() int { return l.size }

func (l *OneLinked) Empty() bool { return l.size == 0 }

func (l *OneLinked) Items() []string {
	out := make([]string, 0, l.size)
	for node := l.head; node != nil; node = node.next {
		out = append(out, node.data)
	}
	return out
}

func (l *OneLinked) String() string { return joinWithSpaces(l.Items()) }

func (l *OneLinked) StringReverse() string { return joinWithSpaces(reversed(l.Items())) }

func (l *OneLinked) Clone() *OneLinked {
	c := &OneLinked{}
	for node := l.head; node != nil; node = node.next {
		c.PushBack(node.data)
	}
	return c
}

func (l *OneLinked) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, "LIN1", l.Items())
}

func (l *OneLinked) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, "LIN1")
	if err != nil {
		return err
	}
	l.replace(items)
	return nil
}

func (l *OneLinked) SerializeText(w io.Writer) error {
	return serializeStringsText(w, "OneLinkedList", l.Items())
}

func (l *OneLinked) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, "OneLinkedList")
	if err != nil {
		return err
	}
	l.replace(items)
	return nil
}

func (l *OneLinked) replace(items []string) {
	l.Clear()
	for _, s := range items {
		l.PushBack(s)
	}
}

func joinWithSpaces(items []string) string {
	var sb strings.Builder
	for _, s := range items {
		sb.WriteString(" ")
		sb.WriteString(s)
	}
	return sb.String()
}

func reversed(items []string) []string {
	out := make([]string, len(items))
	for i, s := range items {
		out[len(items)-1-i] = s
	}
	return out
}
