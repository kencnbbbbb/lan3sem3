package ds

import (
	"bufio"
	"fmt"
	"io"
	"strings"
)

const (
	ArrayMinCap = 16
	ArrayMaxCap = 256 * 256
)

type Array struct {
	data []string
	size int
}

func NewArray(capacity int) (*Array, error) {
	if capacity > ArrayMaxCap {
		return nil, fmt.Errorf("%w: %d", ErrCapacity, capacity)
	}
	if capacity < ArrayMinCap {
		capacity = ArrayMinCap
	}
	return &Array{data: make([]string, capacity)}, nil
}

func (a *Array) grow() error {
	newCap := len(a.data) * 2
	if newCap == 0 {
		newCap = ArrayMinCap
	}
	if newCap > ArrayMaxCap {
		return ErrCapacity
	}
	fresh := make([]string, newCap)
	copy(fresh, a.data[:a.size])
	a.data = fresh
	return nil
}

func (a *Array) ensureSpace() error {
	if a.size >= len(a.data) {
		return a.grow()
	}
	return nil
}

func (a *Array) checkIndex(idx int) error {
	if idx < 0 || idx >= a.size {
		return fmt.Errorf("%w: %d (размер %d)", ErrOutOfRange, idx, a.size)
	}
	return nil
}

func (a *Array) PushBack(v string) error {
	if err := a.ensureSpace(); err != nil {
		return err
	}
	a.data[a.size] = v
	a.size++
	return nil
}

func (a *Array) Insert(idx int, v string) error {
	if idx < 0 || idx > a.size {
		return fmt.Errorf("%w: %d (размер %d)", ErrOutOfRange, idx, a.size)
	}
	if err := a.ensureSpace(); err != nil {
		return err
	}
	copy(a.data[idx+1:a.size+1], a.data[idx:a.size])
	a.data[idx] = v
	a.size++
	return nil
}

func (a *Array) Get(idx int) (string, error) {
	if err := a.checkIndex(idx); err != nil {
		return "", err
	}
	return a.data[idx], nil
}

func (a *Array) Set(idx int, v string) error {
	if err := a.checkIndex(idx); err != nil {
		return err
	}
	a.data[idx] = v
	return nil
}

func (a *Array) Erase(idx int) error {
	if err := a.checkIndex(idx); err != nil {
		return err
	}
	copy(a.data[idx:a.size-1], a.data[idx+1:a.size])
	a.size--
	a.data[a.size] = ""
	return nil
}

func (a *Array) Clear() {
	for i := 0; i < a.size; i++ {
		a.data[i] = ""
	}
	a.size = 0
}

func (a *Array) Len() int { return a.size }

func (a *Array) Cap() int { return len(a.data) }

func (a *Array) Empty() bool { return a.size == 0 }

func (a *Array) Items() []string {
	out := make([]string, a.size)
	copy(out, a.data[:a.size])
	return out
}

func (a *Array) String() string {
	var sb strings.Builder
	for i := 0; i < a.size; i++ {
		sb.WriteString(" ")
		sb.WriteString(a.data[i])
	}
	return sb.String()
}

func (a *Array) Clone() *Array {
	c := &Array{data: make([]string, len(a.data)), size: a.size}
	copy(c.data, a.data)
	return c
}

func (a *Array) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, "ARRY", a.data[:a.size])
}

func (a *Array) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, "ARRY")
	if err != nil {
		return err
	}
	return a.replace(items)
}

func (a *Array) SerializeText(w io.Writer) error {
	return serializeStringsText(w, "Array", a.data[:a.size])
}

func (a *Array) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, "Array")
	if err != nil {
		return err
	}
	return a.replace(items)
}

func (a *Array) replace(items []string) error {
	fresh := &Array{}
	for _, s := range items {
		if err := fresh.PushBack(s); err != nil {
			return err
		}
	}
	*a = *fresh
	return nil
}
