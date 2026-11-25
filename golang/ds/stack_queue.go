package ds

import (
	"bufio"
	"io"
)

type Stack struct {
	head *oneNode
	size int
}

func (s *Stack) Push(v string) {
	s.head = &oneNode{data: v, next: s.head}
	s.size++
}

func (s *Stack) Pop() (string, error) {
	if s.head == nil {
		return "", ErrEmpty
	}
	v := s.head.data
	s.head = s.head.next
	s.size--
	return v, nil
}

func (s *Stack) Top() (string, error) {
	if s.head == nil {
		return "", ErrEmpty
	}
	return s.head.data, nil
}

func (s *Stack) Clear() { *s = Stack{} }

func (s *Stack) Len() int { return s.size }

func (s *Stack) Empty() bool { return s.head == nil }

func (s *Stack) Items() []string {
	out := make([]string, 0, s.size)
	for node := s.head; node != nil; node = node.next {
		out = append(out, node.data)
	}
	return out
}

func (s *Stack) String() string { return joinWithSpaces(s.Items()) }

func (s *Stack) Clone() *Stack {
	c := &Stack{}
	c.fromBottom(reversed(s.Items()))
	return c
}

func (s *Stack) fromBottom(items []string) {
	s.Clear()
	for _, v := range items {
		s.Push(v)
	}
}

func (s *Stack) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, "STAK", reversed(s.Items()))
}

func (s *Stack) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, "STAK")
	if err != nil {
		return err
	}
	s.fromBottom(items)
	return nil
}

func (s *Stack) SerializeText(w io.Writer) error {
	return serializeStringsText(w, "Stack", reversed(s.Items()))
}

func (s *Stack) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, "Stack")
	if err != nil {
		return err
	}
	s.fromBottom(items)
	return nil
}

type Queue struct {
	head *oneNode
	tail *oneNode
	size int
}

func (q *Queue) Push(v string) {
	node := &oneNode{data: v}
	if q.tail != nil {
		q.tail.next = node
	} else {
		q.head = node
	}
	q.tail = node
	q.size++
}

func (q *Queue) Pop() (string, error) {
	if q.head == nil {
		return "", ErrEmpty
	}
	v := q.head.data
	q.head = q.head.next
	if q.head == nil {
		q.tail = nil
	}
	q.size--
	return v, nil
}

func (q *Queue) Front() (string, error) {
	if q.head == nil {
		return "", ErrEmpty
	}
	return q.head.data, nil
}

func (q *Queue) Clear() { *q = Queue{} }

func (q *Queue) Len() int { return q.size }

func (q *Queue) Empty() bool { return q.head == nil }

func (q *Queue) Items() []string {
	out := make([]string, 0, q.size)
	for node := q.head; node != nil; node = node.next {
		out = append(out, node.data)
	}
	return out
}

func (q *Queue) String() string { return joinWithSpaces(q.Items()) }

func (q *Queue) Clone() *Queue {
	c := &Queue{}
	c.replace(q.Items())
	return c
}

func (q *Queue) replace(items []string) {
	q.Clear()
	for _, v := range items {
		q.Push(v)
	}
}

func (q *Queue) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, "QUEU", q.Items())
}

func (q *Queue) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, "QUEU")
	if err != nil {
		return err
	}
	q.replace(items)
	return nil
}

func (q *Queue) SerializeText(w io.Writer) error {
	return serializeStringsText(w, "Queue", q.Items())
}

func (q *Queue) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, "Queue")
	if err != nil {
		return err
	}
	q.replace(items)
	return nil
}
