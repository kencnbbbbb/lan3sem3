package ds

import (
	"bufio"
	"bytes"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"io"
	"math/rand"
	"strconv"
	"strings"
	"testing"
)

func TestBothHashMethods(t *testing.T) {
	for _, makeHash := range []func(int) (*OpenHash, error){NewOpenHash, NewLinearHash} {
		h, e := makeHash(17)
		require.NoError(t, e)
		reference := map[string]bool{}
		rng := rand.New(rand.NewSource(5))
		for i := 0; i < 5000; i++ {
			key := strconv.Itoa(rng.Intn(300))
			switch rng.Intn(3) {
			case 0:
				added, e := h.Insert(key)
				require.NoError(t, e)
				assert.Equal(t, !reference[key], added)
				reference[key] = true
			case 1:
				assert.Equal(t, reference[key], h.Erase(key))
				delete(reference, key)
			case 2:
				assert.Equal(t, reference[key], h.Contains(key))
			}
		}
		assert.Equal(t, len(reference), h.Len())
		assert.Equal(t, h.Method(), h.Clone().Method())
		for _, binary := range []bool{true, false} {
			copy, e := makeHash(0)
			require.NoError(t, e)
			var out bytes.Buffer
			if binary {
				require.NoError(t, h.SerializeBinary(&out))
				require.NoError(t, copy.DeserializeBinary(&out))
			} else {
				require.NoError(t, h.SerializeText(&out))
				require.NoError(t, copy.DeserializeText(bufio.NewReader(&out)))
			}
			assert.ElementsMatch(t, h.Items(), copy.Items())
			assert.Equal(t, h.Method(), copy.Method())
		}
	}
	_, e := NewLinearHash(OpenHashMaxCap + 1)
	require.Error(t, e)
}
func TestDeserializeAtomic(t *testing.T) {
	for name, mk := range serializerFactories() {
		t.Run(name, func(t *testing.T) {
			obj := mk()
			var empty bytes.Buffer
			require.NoError(t, obj.SerializeBinary(&empty))
			raw := append([]byte(nil), empty.Bytes()...)
			for cut := 0; cut < len(raw); cut++ {
				before := obj.String()
				assert.Error(t, obj.DeserializeBinary(bytes.NewReader(raw[:cut])))
				assert.Equal(t, before, obj.String())
			}
		})
	}
}

// Частичная запись и ошибки ввода не должны выглядеть успешной сериализацией.
type failingWriter struct{ left int }

func (w *failingWriter) Write(p []byte) (int, error) {
	n := len(p)
	if n > w.left {
		n = w.left
	}
	w.left -= n
	if n < len(p) {
		return n, io.ErrClosedPipe
	}
	return n, nil
}
func TestSerializationIOFailures(t *testing.T) {
	for name, mk := range serializerFactories() {
		t.Run(name, func(t *testing.T) {
			obj := mk()
			switch v := obj.(type) {
			case *Array:
				require.NoError(t, v.PushBack("two words"))
			case *OneLinked:
				v.PushBack("two words")
			case *Linked:
				v.PushBack("two words")
			case *Stack:
				v.Push("two words")
			case *Queue:
				v.Push("two words")
			case *OpenHash:
				_, e := v.Insert("two words")
				require.NoError(t, e)
			case *Tree:
				v.Insert(-7)
			}
			var binary, text bytes.Buffer
			require.NoError(t, obj.SerializeBinary(&binary))
			require.NoError(t, obj.SerializeText(&text))
			for budget := 0; budget < binary.Len(); budget++ {
				assert.Error(t, obj.SerializeBinary(&failingWriter{budget}))
			}
			for budget := 0; budget < text.Len(); budget++ {
				assert.Error(t, obj.SerializeText(&failingWriter{budget}))
			}
			before := obj.String()
			for cut := 0; cut < binary.Len(); cut++ {
				assert.Error(t, obj.DeserializeBinary(bytes.NewReader(binary.Bytes()[:cut])))
				assert.Equal(t, before, obj.String())
			}
		})
	}
	assert.Error(t, WriteTag(&shortWriter{}, "ABCD"))
	assert.Error(t, WriteString(&shortWriter{}, "abc"))
	tr := &Tree{}
	assert.False(t, tr.Insert(1<<40))
	assert.True(t, tr.Empty())
	assert.Error(t, tr.DeserializeText(bufio.NewReader(strings.NewReader("CompleteTree 1\n2147483648\n"))))
}

type shortWriter struct{}

func (*shortWriter) Write(p []byte) (int, error) { return 0, nil }
