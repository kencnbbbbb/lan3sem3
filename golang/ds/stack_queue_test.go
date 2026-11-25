package ds

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestStackLifo(t *testing.T) {
	s := &Stack{}
	assert.True(t, s.Empty())
	s.Push("a")
	s.Push("b")
	top, err := s.Top()
	require.NoError(t, err)
	assert.Equal(t, "b", top)
	assert.Equal(t, " b a", s.String())
	assert.Equal(t, 2, s.Len())
	v, err := s.Pop()
	require.NoError(t, err)
	assert.Equal(t, "b", v)
	v, err = s.Pop()
	require.NoError(t, err)
	assert.Equal(t, "a", v)
	_, err = s.Pop()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = s.Top()
	assert.ErrorIs(t, err, ErrEmpty)
}

func TestStackCloneClear(t *testing.T) {
	s := &Stack{}
	s.Push("a")
	s.Push("b")
	c := s.Clone()
	c.Push("c")
	assert.Equal(t, " b a", s.String())
	assert.Equal(t, " c b a", c.String())
	s.Clear()
	assert.True(t, s.Empty())
}

func TestQueueFifo(t *testing.T) {
	q := &Queue{}
	assert.True(t, q.Empty())
	q.Push("a")
	q.Push("b")
	q.Push("c")
	front, err := q.Front()
	require.NoError(t, err)
	assert.Equal(t, "a", front)
	assert.Equal(t, " a b c", q.String())
	v, _ := q.Pop()
	assert.Equal(t, "a", v)
	v, _ = q.Pop()
	assert.Equal(t, "b", v)
	q.Push("d")
	v, _ = q.Pop()
	assert.Equal(t, "c", v)
	v, _ = q.Pop()
	assert.Equal(t, "d", v)
	_, err = q.Pop()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = q.Front()
	assert.ErrorIs(t, err, ErrEmpty)
	q.Push("e")
	assert.Equal(t, " e", q.String())
}

func TestQueueCloneClear(t *testing.T) {
	q := &Queue{}
	q.Push("a")
	q.Push("b")
	c := q.Clone()
	c.Push("c")
	assert.Equal(t, " a b", q.String())
	assert.Equal(t, " a b c", c.String())
	assert.Equal(t, 2, q.Len())
	q.Clear()
	assert.True(t, q.Empty())
}
