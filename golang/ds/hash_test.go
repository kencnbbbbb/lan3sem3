package ds

import (
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestOpenHashBasics(t *testing.T) {
	h, err := NewOpenHash(0)
	require.NoError(t, err)
	ok, err := h.Insert("a")
	require.NoError(t, err)
	assert.True(t, ok)
	ok, _ = h.Insert("b")
	assert.True(t, ok)
	ok, _ = h.Insert("a")
	assert.False(t, ok)
	assert.True(t, h.Contains("a"))
	assert.False(t, h.Contains("zzz"))
	assert.Equal(t, 2, h.Len())
	assert.GreaterOrEqual(t, h.Find("a"), 0)
	assert.Equal(t, -1, h.Find("zzz"))
	assert.True(t, h.Erase("a"))
	assert.False(t, h.Erase("a"))
	assert.Equal(t, -1, h.Find("a"))
	ok, _ = h.Insert("a")
	assert.True(t, ok)
	h.Clear()
	assert.True(t, h.Empty())
	assert.Equal(t, "", h.String())
}

func TestOpenHashRehash(t *testing.T) {
	h, err := NewOpenHash(0)
	require.NoError(t, err)
	for i := 0; i < 500; i++ {
		ok, err := h.Insert("k" + strconv.Itoa(i))
		require.NoError(t, err)
		require.True(t, ok)
	}
	assert.Equal(t, 500, h.Len())
	assert.Greater(t, h.Cap(), OpenHashMinCap)
	for i := 0; i < 500; i += 2 {
		assert.True(t, h.Erase("k"+strconv.Itoa(i)))
	}
	for i := 0; i < 500; i++ {
		assert.Equal(t, i%2 == 1, h.Contains("k"+strconv.Itoa(i)))
	}
	assert.Len(t, h.Items(), 250)
}

func TestOpenHashTombstones(t *testing.T) {
	h, err := NewOpenHash(0)
	require.NoError(t, err)
	for i := 0; i < 50; i++ {
		_, err := h.Insert("t" + strconv.Itoa(i))
		require.NoError(t, err)
		h.Erase("t" + strconv.Itoa(i))
	}
	assert.True(t, h.Empty())
}

func TestOpenHashLimits(t *testing.T) {
	_, err := NewOpenHash(OpenHashMaxCap + 1)
	assert.ErrorIs(t, err, ErrCapacity)
	h, err := NewOpenHash(20)
	require.NoError(t, err)
	assert.Equal(t, 32, h.Cap())
	big, err := NewOpenHash(OpenHashMaxCap)
	require.NoError(t, err)
	var gotErr error
	for i := 0; i < 60000 && gotErr == nil; i++ {
		_, gotErr = big.Insert("k" + strconv.Itoa(i))
	}
	assert.ErrorIs(t, gotErr, ErrCapacity)
}

func TestHashFunctions(t *testing.T) {
	assert.Equal(t, HashDJB2("abc"), HashDJB2("abc"))
	assert.NotEqual(t, HashDJB2("abc"), HashDJB2("abd"))
	assert.Equal(t, HashFNV1a("abc"), HashFNV1a("abc"))
	assert.NotEqual(t, HashFNV1a("abc"), HashFNV1a("abd"))
}

func TestOpenHashClone(t *testing.T) {
	h, err := NewOpenHash(0)
	require.NoError(t, err)
	_, _ = h.Insert("a")
	c := h.Clone()
	_, _ = c.Insert("b")
	assert.Equal(t, 1, h.Len())
	assert.Equal(t, 2, c.Len())
	assert.False(t, h.Contains("b"))
}
