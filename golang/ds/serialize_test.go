package ds

import (
	"bufio"
	"bytes"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

type serialized interface {
	Serializer
	Empty() bool
	String() string
}

func serializerFactories() map[string]func() serialized {
	return map[string]func() serialized{
		"Array":      func() serialized { a, _ := NewArray(0); return a },
		"OneLinked":  func() serialized { return &OneLinked{} },
		"Linked":     func() serialized { return &Linked{} },
		"Stack":      func() serialized { return &Stack{} },
		"Queue":      func() serialized { return &Queue{} },
		"OpenHash":   func() serialized { h, _ := NewOpenHash(0); return h },
		"LinearHash": func() serialized { h, _ := NewLinearHash(0); return h },
		"Tree":       func() serialized { return &Tree{} },
	}
}

func TestSerializeEmptyRoundTrip(t *testing.T) {
	for name, mk := range serializerFactories() {
		t.Run(name, func(t *testing.T) {
			var bin bytes.Buffer
			require.NoError(t, mk().SerializeBinary(&bin))
			b := mk()
			require.NoError(t, b.DeserializeBinary(&bin))
			assert.True(t, b.Empty())
			var txt bytes.Buffer
			require.NoError(t, mk().SerializeText(&txt))
			c := mk()
			require.NoError(t, c.DeserializeText(bufio.NewReader(&txt)))
			assert.True(t, c.Empty())
		})
	}
}

func TestSerializeBadBinary(t *testing.T) {
	for name, mk := range serializerFactories() {
		t.Run(name, func(t *testing.T) {
			assert.ErrorIs(t, mk().DeserializeBinary(strings.NewReader("XXXXXXXXXXXX")), ErrFormat)
			assert.ErrorIs(t, mk().DeserializeBinary(strings.NewReader("")), ErrFormat)
			var ok bytes.Buffer
			require.NoError(t, mk().SerializeBinary(&ok))
			raw := ok.Bytes()
			assert.ErrorIs(t, mk().DeserializeBinary(bytes.NewReader(raw[:6])), ErrFormat)
			huge := append([]byte{}, raw...)
			for i := 4; i < 12; i++ {
				huge[i] = 0xFF
			}
			assert.ErrorIs(t, mk().DeserializeBinary(bytes.NewReader(huge)), ErrFormat)
		})
	}
}

func TestSerializeBadText(t *testing.T) {
	for name, mk := range serializerFactories() {
		t.Run(name, func(t *testing.T) {
			assert.ErrorIs(t, mk().DeserializeText(bufio.NewReader(strings.NewReader("Wrong 0\n"))), ErrFormat)
			assert.ErrorIs(t, mk().DeserializeText(bufio.NewReader(strings.NewReader(""))), ErrFormat)
			var ok bytes.Buffer
			require.NoError(t, mk().SerializeText(&ok))
			head := strings.SplitN(ok.String(), " ", 2)[0]
			assert.ErrorIs(t, mk().DeserializeText(bufio.NewReader(strings.NewReader(head+" 99999999999\n"))), ErrFormat)
			assert.ErrorIs(t, mk().DeserializeText(bufio.NewReader(strings.NewReader(head+" 3\n"))), ErrFormat)
			assert.ErrorIs(t, mk().DeserializeText(bufio.NewReader(strings.NewReader(head+" -1\n"))), ErrFormat)
		})
	}
}

func roundTrip(t *testing.T, src, dst Serializer) {
	t.Helper()
	var bin bytes.Buffer
	require.NoError(t, src.SerializeBinary(&bin))
	require.NoError(t, dst.DeserializeBinary(&bin))
}

func roundTripText(t *testing.T, src, dst Serializer) {
	t.Helper()
	var txt bytes.Buffer
	require.NoError(t, src.SerializeText(&txt))
	require.NoError(t, dst.DeserializeText(bufio.NewReader(&txt)))
}

func TestSerializeFilledRoundTrip(t *testing.T) {
	arr, _ := NewArray(0)
	for _, v := range []string{"a", "two words", "q\"uote", ""} {
		require.NoError(t, arr.PushBack(v))
	}
	one := filled(&OneLinked{})
	two := filled(&Linked{})
	st := &Stack{}
	st.Push("a")
	st.Push("two words")
	qu := &Queue{}
	qu.Push("a")
	qu.Push("two words")
	hash, _ := NewOpenHash(0)
	_, _ = hash.Insert("a")
	_, _ = hash.Insert("two words")
	tree := newFilledTree()

	pairs := []struct {
		name     string
		src, dst serialized
	}{
		{"Array", arr, func() serialized { a, _ := NewArray(0); _ = a.PushBack("old"); return a }()},
		{"OneLinked", one.(serialized), &OneLinked{}},
		{"Linked", two.(serialized), &Linked{}},
		{"Stack", st, &Stack{}},
		{"Queue", qu, &Queue{}},
		{"Tree", tree, &Tree{}},
	}
	for _, p := range pairs {
		t.Run(p.name+"/binary", func(t *testing.T) {
			roundTrip(t, p.src, p.dst)
			assert.Equal(t, p.src.String(), p.dst.String())
		})
	}
	for _, p := range pairs {
		t.Run(p.name+"/text", func(t *testing.T) {
			dst := p.dst
			dst.(interface{ Clear() }).Clear()
			roundTripText(t, p.src, dst)
			assert.Equal(t, p.src.String(), dst.String())
		})
	}

	t.Run("OpenHash", func(t *testing.T) {
		b, _ := NewOpenHash(0)
		roundTrip(t, hash, b)
		assert.True(t, b.Contains("two words"))
		assert.Equal(t, 2, b.Len())
		c, _ := NewOpenHash(0)
		roundTripText(t, hash, c)
		assert.True(t, c.Contains("a"))
		assert.Equal(t, 2, c.Len())
	})
}

func TestPeekTags(t *testing.T) {
	tag, err := PeekTextTag(bufio.NewReader(strings.NewReader("  Stack 2\n")))
	require.NoError(t, err)
	assert.Equal(t, "Stack", tag)
	_, err = PeekTextTag(bufio.NewReader(strings.NewReader("   ")))
	assert.ErrorIs(t, err, ErrFormat)
	btag, err := PeekBinaryTag(bufio.NewReader(strings.NewReader("STAKxxxx")))
	require.NoError(t, err)
	assert.Equal(t, "STAK", btag)
	_, err = PeekBinaryTag(bufio.NewReader(strings.NewReader("ST")))
	assert.ErrorIs(t, err, ErrFormat)
}

func TestPrimitives(t *testing.T) {
	var buf bytes.Buffer
	require.NoError(t, WriteTag(&buf, "ABCD"))
	require.NoError(t, WriteSize(&buf, 7))
	require.NoError(t, WriteString(&buf, "hi"))
	require.NoError(t, ReadTag(&buf, "ABCD"))
	n, err := ReadSize(&buf)
	require.NoError(t, err)
	assert.Equal(t, 7, n)
	s, err := ReadString(&buf)
	require.NoError(t, err)
	assert.Equal(t, "hi", s)

	var txt bytes.Buffer
	require.NoError(t, WriteTextHeader(&txt, "Tag", 2))
	require.NoError(t, WriteTextString(&txt, "a\"b\\c"))
	r := bufio.NewReader(&txt)
	cnt, err := ReadTextHeader(r, "Tag")
	require.NoError(t, err)
	assert.Equal(t, 2, cnt)
	str, err := ReadTextString(r)
	require.NoError(t, err)
	assert.Equal(t, "a\"b\\c", str)

	_, err = ReadTextString(bufio.NewReader(strings.NewReader("abc")))
	assert.ErrorIs(t, err, ErrFormat)
	_, err = ReadTextString(bufio.NewReader(strings.NewReader("\"abc")))
	assert.ErrorIs(t, err, ErrFormat)
	_, err = ReadTextString(bufio.NewReader(strings.NewReader("\"ab\\")))
	assert.ErrorIs(t, err, ErrFormat)
	_, err = ReadTextString(bufio.NewReader(strings.NewReader("")))
	assert.ErrorIs(t, err, ErrFormat)
	assert.Error(t, ReadTag(strings.NewReader("ABCD"), "WXYZ"))
	_, err = ReadTextHeader(bufio.NewReader(strings.NewReader("Tag x\n")), "Tag")
	assert.ErrorIs(t, err, ErrFormat)
}

func TestWriteIntOverflow(t *testing.T) {
	var buf bytes.Buffer
	assert.ErrorIs(t, writeInt(&buf, 1<<40), ErrFormat)
	require.NoError(t, writeInt(&buf, -5))
	v, err := readInt(&buf)
	require.NoError(t, err)
	assert.Equal(t, -5, v)
	_, err = readInt(&buf)
	assert.ErrorIs(t, err, ErrFormat)
}
