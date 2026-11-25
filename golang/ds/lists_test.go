package ds

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

type stringList interface {
	Find(value string) int
	PushBack(v string)
	PushFront(v string)
	PushAfter(prev, v string) error
	PushBefore(next, v string) error
	PopFront() (string, error)
	PopBack() (string, error)
	PopAfter(prev string) (string, error)
	PopBefore(next string) (string, error)
	DelByValue(value string) bool
	Clear()
	Len() int
	Empty() bool
	Items() []string
	String() string
	StringReverse() string
}

func listFactories() map[string]func() stringList {
	return map[string]func() stringList{
		"OneLinked": func() stringList { return &OneLinked{} },
		"Linked":    func() stringList { return &Linked{} },
	}
}

func filled(l stringList) stringList {
	for _, v := range []string{"10", "20", "30", "40", "50"} {
		l.PushBack(v)
	}
	return l
}

func TestListsPushAndRead(t *testing.T) {
	for name, mk := range listFactories() {
		t.Run(name, func(t *testing.T) {
			l := mk()
			assert.True(t, l.Empty())
			assert.Equal(t, "", l.String())
			assert.Equal(t, "", l.StringReverse())
			l.PushBack("b")
			l.PushFront("a")
			l.PushBack("c")
			assert.Equal(t, " a b c", l.String())
			assert.Equal(t, " c b a", l.StringReverse())
			assert.Equal(t, 3, l.Len())
			assert.Equal(t, 1, l.Find("b"))
			assert.Equal(t, -1, l.Find("zzz"))
		})
	}
}

func TestListsPushAfterBefore(t *testing.T) {
	for name, mk := range listFactories() {
		t.Run(name, func(t *testing.T) {
			l := filled(mk())
			require.NoError(t, l.PushAfter("50", "60"))
			require.NoError(t, l.PushAfter("10", "15"))
			require.NoError(t, l.PushBefore("10", "05"))
			require.NoError(t, l.PushBefore("30", "25"))
			assert.Equal(t, " 05 10 15 20 25 30 40 50 60", l.String())
			assert.Equal(t, " 60 50 40 30 25 20 15 10 05", l.StringReverse())
			assert.ErrorIs(t, l.PushAfter("zzz", "1"), ErrNotFound)
			assert.ErrorIs(t, l.PushBefore("zzz", "1"), ErrNotFound)
		})
	}
}

func TestListsPops(t *testing.T) {
	for name, mk := range listFactories() {
		t.Run(name, func(t *testing.T) {
			l := filled(mk())
			v, err := l.PopFront()
			require.NoError(t, err)
			assert.Equal(t, "10", v)
			v, err = l.PopBack()
			require.NoError(t, err)
			assert.Equal(t, "50", v)
			v, err = l.PopAfter("20")
			require.NoError(t, err)
			assert.Equal(t, "30", v)
			v, err = l.PopBefore("40")
			require.NoError(t, err)
			assert.Equal(t, "20", v)
			assert.Equal(t, " 40", l.String())
			_, err = l.PopAfter("40")
			assert.ErrorIs(t, err, ErrNoNeighbor)
			_, err = l.PopBefore("40")
			assert.ErrorIs(t, err, ErrNoNeighbor)
			_, err = l.PopAfter("zzz")
			assert.ErrorIs(t, err, ErrNotFound)
			_, err = l.PopBefore("zzz")
			assert.ErrorIs(t, err, ErrNotFound)
			_, err = l.PopBack()
			require.NoError(t, err)
			_, err = l.PopFront()
			assert.ErrorIs(t, err, ErrEmpty)
			_, err = l.PopBack()
			assert.ErrorIs(t, err, ErrEmpty)
		})
	}
}

func TestListsDelAndClear(t *testing.T) {
	for name, mk := range listFactories() {
		t.Run(name, func(t *testing.T) {
			l := filled(mk())
			assert.True(t, l.DelByValue("10"))
			assert.True(t, l.DelByValue("50"))
			assert.True(t, l.DelByValue("30"))
			assert.False(t, l.DelByValue("zzz"))
			assert.Equal(t, " 20 40", l.String())
			l.Clear()
			assert.True(t, l.Empty())
			l.PushFront("n")
			assert.Equal(t, " n", l.String())
		})
	}
}

func TestOneLinkedClone(t *testing.T) {
	l := filled(&OneLinked{}).(*OneLinked)
	c := l.Clone()
	c.PushBack("60")
	assert.Equal(t, " 10 20 30 40 50", l.String())
	assert.Equal(t, " 10 20 30 40 50 60", c.String())
}

func TestLinkedClone(t *testing.T) {
	l := filled(&Linked{}).(*Linked)
	c := l.Clone()
	c.PushBack("60")
	assert.Equal(t, " 10 20 30 40 50", l.String())
	assert.Equal(t, " 10 20 30 40 50 60", c.String())
}
