package ds

import (
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func newFilledArray(t *testing.T) *Array {
	t.Helper()
	a, err := NewArray(0)
	require.NoError(t, err)
	for _, v := range []string{"a", "b", "c"} {
		require.NoError(t, a.PushBack(v))
	}
	return a
}

func TestArrayGetSet(t *testing.T) {
	a := newFilledArray(t)
	assert.Equal(t, 3, a.Len())
	assert.Equal(t, " a b c", a.String())
	v, err := a.Get(2)
	require.NoError(t, err)
	assert.Equal(t, "c", v)
	_, err = a.Get(3)
	assert.ErrorIs(t, err, ErrOutOfRange)
	require.NoError(t, a.Set(1, "B"))
	assert.Equal(t, " a B c", a.String())
	assert.ErrorIs(t, a.Set(3, "x"), ErrOutOfRange)
	assert.ErrorIs(t, a.Set(-1, "x"), ErrOutOfRange)
}

func TestArrayInsertErase(t *testing.T) {
	a := newFilledArray(t)
	require.NoError(t, a.Insert(0, "x"))
	require.NoError(t, a.Insert(2, "y"))
	require.NoError(t, a.Insert(a.Len(), "z"))
	assert.Equal(t, " x a y b c z", a.String())
	assert.ErrorIs(t, a.Insert(a.Len()+1, "bad"), ErrOutOfRange)
	require.NoError(t, a.Erase(0))
	require.NoError(t, a.Erase(a.Len()-1))
	assert.Equal(t, " a y b c", a.String())
	assert.ErrorIs(t, a.Erase(a.Len()), ErrOutOfRange)
	a.Clear()
	assert.True(t, a.Empty())
}

func TestArrayCapacity(t *testing.T) {
	a, err := NewArray(0)
	require.NoError(t, err)
	assert.Equal(t, ArrayMinCap, a.Cap())
	for i := 0; i < ArrayMinCap+1; i++ {
		require.NoError(t, a.PushBack(strconv.Itoa(i)))
	}
	assert.Equal(t, ArrayMinCap*2, a.Cap())
	_, err = NewArray(ArrayMaxCap + 1)
	assert.ErrorIs(t, err, ErrCapacity)
	big, err := NewArray(ArrayMaxCap)
	require.NoError(t, err)
	for i := 0; i < ArrayMaxCap; i++ {
		require.NoError(t, big.PushBack("x"))
	}
	assert.ErrorIs(t, big.PushBack("x"), ErrCapacity)
	assert.ErrorIs(t, big.Insert(0, "x"), ErrCapacity)
}

func TestArrayClone(t *testing.T) {
	a := newFilledArray(t)
	c := a.Clone()
	require.NoError(t, c.Set(0, "X"))
	assert.Equal(t, " a b c", a.String())
	assert.Equal(t, " X b c", c.String())
	assert.Equal(t, []string{"a", "b", "c"}, a.Items())
}
