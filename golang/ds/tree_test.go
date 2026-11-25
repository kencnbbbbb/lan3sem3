package ds

import (
	"bufio"
	"bytes"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"testing"
)

func newFilledTree() *Tree {
	t := &Tree{}
	for _, v := range []int{50, 30, 70, 20, 40, 60, 80} {
		t.Insert(v)
	}
	return t
}
func TestCompleteTreeOperations(t *testing.T) {
	tr := &Tree{}
	assert.True(t, tr.Empty())
	assert.True(t, tr.IsComplete())
	assert.Equal(t, 0, tr.Height())
	assert.False(t, tr.IsMember(1))
	assert.Empty(t, tr.Structure())
	for i := 0; i < 100; i++ {
		require.True(t, tr.Insert(i%7))
		assert.True(t, tr.IsComplete())
		assert.Equal(t, i+1, tr.Len())
	}
	assert.True(t, tr.IsMember(6))
	assert.False(t, tr.IsMember(-1))
	assert.Equal(t, 7, tr.Height())
	tr = newFilledTree()
	assert.Equal(t, " 50 30 70 20 40 60 80", tr.String())
	assert.Equal(t, []int{50, 30, 70, 20, 40, 60, 80}, tr.Items())
	assert.Contains(t, tr.Structure(), "root->right->right = 80")
	require.True(t, tr.Insert(50))
	assert.Equal(t, 8, tr.Len())
	assert.Equal(t, 4, tr.Height())
	assert.Contains(t, tr.Structure(), "root->left->left->left = 50")
	c := tr.Clone()
	c.Insert(999)
	assert.False(t, tr.IsMember(999))
	tr.Clear()
	assert.True(t, tr.Empty())
	assert.Equal(t, 0, tr.Len())
	assert.NotEmpty(t, c.String())
}
func TestCompleteTreeMalformedShapes(t *testing.T) {
	tr := &Tree{root: &treeNode{data: 1, right: &treeNode{data: 2}}, size: 2}
	assert.False(t, tr.IsComplete())
	tr = &Tree{root: &treeNode{data: 1, left: &treeNode{data: 2}, right: &treeNode{data: 3, left: &treeNode{data: 4}}}, size: 4}
	assert.False(t, tr.IsComplete())
}
func TestCompleteTreeRoundTripAndFailure(t *testing.T) {
	src := newFilledTree()
	src.Insert(50)
	var b bytes.Buffer
	require.NoError(t, src.SerializeBinary(&b))
	dst := &Tree{}
	dst.Insert(-1)
	require.NoError(t, dst.DeserializeBinary(&b))
	assert.Equal(t, src.Structure(), dst.Structure())
	assert.True(t, dst.IsComplete())
	var txt bytes.Buffer
	require.NoError(t, src.SerializeText(&txt))
	require.NoError(t, dst.DeserializeText(bufio.NewReader(&txt)))
	assert.Equal(t, src.Structure(), dst.Structure())
	before := dst.Structure()
	assert.Error(t, dst.DeserializeText(bufio.NewReader(bytes.NewBufferString("CompleteTree 2\n1\n"))))
	assert.Equal(t, before, dst.Structure())
}
