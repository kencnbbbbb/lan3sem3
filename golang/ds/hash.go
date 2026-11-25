package ds

import (
	"bufio"
	"io"
)

func HashDJB2(key string) uint64 {
	var h uint64 = 5381
	for i := 0; i < len(key); i++ {
		h = (h << 5) + h + uint64(key[i])
	}
	return h
}

func HashFNV1a(key string) uint64 {
	var h uint64 = 14695981039346656037
	for i := 0; i < len(key); i++ {
		h ^= uint64(key[i])
		h *= 1099511628211
	}
	return h
}

const (
	OpenHashMinCap = 16
	OpenHashMaxCap = 256 * 256
	openMaxLoad    = 0.7
)

type slotStatus uint8

const (
	slotEmpty slotStatus = iota
	slotUsed
	slotDeleted
)

type slot struct {
	status slotStatus
	val    string
}

type OpenHash struct {
	linear  bool
	slots   []slot
	size    int
	deleted int
}

func NewOpenHash(capacity int) (*OpenHash, error) {
	if capacity > OpenHashMaxCap {
		return nil, ErrCapacity
	}
	c := OpenHashMinCap
	for c < capacity {
		c <<= 1
	}
	return &OpenHash{slots: make([]slot, c)}, nil
}

func (h *OpenHash) probe(key string, i uint64) int {
	mask := uint64(len(h.slots) - 1)
	step := HashFNV1a(key) | 1
	if h.linear {
		step = 1
	}
	return int((HashDJB2(key) + i*step) & mask)
}

func (h *OpenHash) Find(value string) int {
	for i := range uint64(len(h.slots)) {
		idx := h.probe(value, i)
		switch {
		case h.slots[idx].status == slotEmpty:
			return -1
		case h.slots[idx].status == slotUsed && h.slots[idx].val == value:
			return idx
		}
	}
	return -1
}

func (h *OpenHash) Contains(value string) bool { return h.Find(value) != -1 }

func (h *OpenHash) place(value string) {
	for i := uint64(0); ; i++ {
		s := &h.slots[h.probe(value, i)]
		if s.status != slotUsed {
			if s.status == slotDeleted {
				h.deleted--
			}
			*s = slot{status: slotUsed, val: value}
			h.size++
			return
		}
	}
}

func (h *OpenHash) rehash(newCap int) error {
	if newCap > OpenHashMaxCap {
		return ErrCapacity
	}
	old := h.slots
	h.slots = make([]slot, newCap)
	h.size, h.deleted = 0, 0
	for _, s := range old {
		if s.status == slotUsed {
			h.place(s.val)
		}
	}
	return nil
}

func (h *OpenHash) Insert(value string) (bool, error) {
	if h.Contains(value) {
		return false, nil
	}
	if len(h.slots) == 0 {
		h.slots = make([]slot, OpenHashMinCap)
	}
	capacity := float64(len(h.slots))
	if float64(h.size+h.deleted+1) > capacity*openMaxLoad {
		newCap := len(h.slots)
		if float64(h.size+1) > capacity*openMaxLoad/2 {
			newCap *= 2
		}
		if err := h.rehash(newCap); err != nil {
			return false, err
		}
	}
	h.place(value)
	return true, nil
}

func (h *OpenHash) Erase(value string) bool {
	idx := h.Find(value)
	if idx == -1 {
		return false
	}
	h.slots[idx] = slot{status: slotDeleted}
	h.size--
	h.deleted++
	return true
}

func (h *OpenHash) Clear() {
	for i := range h.slots {
		h.slots[i] = slot{}
	}
	h.size, h.deleted = 0, 0
}

func (h *OpenHash) Len() int { return h.size }

func (h *OpenHash) Cap() int { return len(h.slots) }

func (h *OpenHash) Empty() bool { return h.size == 0 }

func (h *OpenHash) Items() []string {
	out := make([]string, 0, h.size)
	for _, s := range h.slots {
		if s.status == slotUsed {
			out = append(out, s.val)
		}
	}
	return out
}

func (h *OpenHash) String() string { return joinWithSpaces(h.Items()) }

func (h *OpenHash) Clone() *OpenHash {
	c := &OpenHash{slots: make([]slot, len(h.slots)), size: h.size, deleted: h.deleted, linear: h.linear}
	copy(c.slots, h.slots)
	return c
}

func (h *OpenHash) replace(items []string) error {
	fresh := &OpenHash{linear: h.linear}
	for _, v := range items {
		if _, err := fresh.Insert(v); err != nil {
			return err
		}
	}
	*h = *fresh
	return nil
}

func (h *OpenHash) SerializeBinary(w io.Writer) error {
	return serializeStringsBinary(w, h.binaryTag(), h.Items())
}

func (h *OpenHash) DeserializeBinary(r io.Reader) error {
	items, err := deserializeStringsBinary(r, h.binaryTag())
	if err != nil {
		return err
	}
	return h.replace(items)
}

func (h *OpenHash) SerializeText(w io.Writer) error {
	return serializeStringsText(w, h.textTag(), h.Items())
}

func (h *OpenHash) DeserializeText(r *bufio.Reader) error {
	items, err := deserializeStringsText(r, h.textTag())
	if err != nil {
		return err
	}
	return h.replace(items)
}

// NewLinearHash использует те же операции с линейным пробированием из ЛР2.
func NewLinearHash(capacity int) (*OpenHash, error) {
	h, e := NewOpenHash(capacity)
	if e == nil {
		h.linear = true
	}
	return h, e
}
func (h *OpenHash) Method() string {
	if h.linear {
		return "linear"
	}
	return "double"
}
func (h *OpenHash) binaryTag() string {
	if h.linear {
		return "HLNR"
	}
	return "HOPN"
}
func (h *OpenHash) textTag() string {
	if h.linear {
		return "LinearHash"
	}
	return "OpenHash"
}
