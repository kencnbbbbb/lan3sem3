package ds

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// Контрольная последовательность из личной ЛР1, вариант дерева 3.
func TestCompleteTreeLab1Example(t *testing.T) {
	tr := &Tree{}
	for _, v := range []int{10, 5, 20, 3, 7, 15} {
		require.True(t, tr.Insert(v))
	}
	assert.Equal(t, []int{10, 5, 20, 3, 7, 15}, tr.Items())
	assert.True(t, tr.IsComplete())
	assert.True(t, tr.IsMember(7))
	assert.False(t, tr.IsMember(100))
}
