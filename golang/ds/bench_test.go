package ds

import (
	"strconv"
	"testing"
)

func benchKeys(n int) []string {
	keys := make([]string, n)
	for i := range keys {
		keys[i] = "key" + strconv.Itoa(i)
	}
	return keys
}

var benchmarkFound bool

var benchSizes = []int{100, 1000, 10000}

func runSizes(b *testing.B, f func(b *testing.B, keys []string)) {
	for _, n := range benchSizes {
		keys := benchKeys(n)
		b.Run("N="+strconv.Itoa(n), func(b *testing.B) { f(b, keys) })
	}
}

func BenchmarkArrayPushBack(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			a, _ := NewArray(0)
			for _, k := range keys {
				_ = a.PushBack(k)
			}
		}
	})
}

func BenchmarkOneLinkedPushBack(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			l := &OneLinked{}
			for _, k := range keys {
				l.PushBack(k)
			}
		}
	})
}

func BenchmarkLinkedPushBack(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			l := &Linked{}
			for _, k := range keys {
				l.PushBack(k)
			}
		}
	})
}

func BenchmarkStackPushPop(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			s := &Stack{}
			for _, k := range keys {
				s.Push(k)
			}
			for !s.Empty() {
				_, _ = s.Pop()
			}
		}
	})
}

func BenchmarkQueuePushPop(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			q := &Queue{}
			for _, k := range keys {
				q.Push(k)
			}
			for !q.Empty() {
				_, _ = q.Pop()
			}
		}
	})
}

func BenchmarkOpenHashInsert(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			h, _ := NewOpenHash(0)
			for _, k := range keys {
				_, _ = h.Insert(k)
			}
		}
	})
}

func BenchmarkOpenHashContains(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		h, _ := NewOpenHash(0)
		for _, k := range keys {
			_, _ = h.Insert(k)
		}
		b.ResetTimer()
		for i := 0; i < b.N; i++ {
			for _, k := range keys {
				benchmarkFound = h.Contains(k)
			}
		}
	})
}

func BenchmarkTreeInsert(b *testing.B) {
	for _, n := range []int{100, 1000} {
		b.Run("N="+strconv.Itoa(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				t := &Tree{}
				for j := 0; j < n; j++ {
					t.Insert((j * 7919) % (n * 10))
				}
			}
		})
	}
}

func BenchmarkTreeMember(b *testing.B) {
	for _, n := range []int{100, 1000} {
		t := &Tree{}
		for j := 0; j < n; j++ {
			t.Insert(j)
		}
		b.Run("N="+strconv.Itoa(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				for j := 0; j < n; j++ {
					benchmarkFound = t.IsMember(j)
				}
			}
		})
	}
}

func BenchmarkLinearHashInsert(b *testing.B) {
	runSizes(b, func(b *testing.B, keys []string) {
		for i := 0; i < b.N; i++ {
			h, _ := NewLinearHash(0)
			for _, key := range keys {
				_, _ = h.Insert(key)
			}
		}
	})
}
