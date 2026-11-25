package ds

import (
	"bufio"
	"fmt"
	"io"
	"math"
	"strconv"
	"strings"
)

type treeNode struct {
	data        int
	left, right *treeNode
}
type Tree struct {
	root *treeNode
	size int
}

func (t *Tree) Insert(v int) bool {
	if v < math.MinInt32 || v > math.MaxInt32 {
		return false
	}
	n := &treeNode{data: v}
	if t.root == nil {
		t.root = n
		t.size++
		return true
	}
	queue := []*treeNode{t.root}
	for i := 0; i < len(queue); i++ {
		cur := queue[i]
		if cur.left == nil {
			cur.left = n
			t.size++
			return true
		}
		if cur.right == nil {
			cur.right = n
			t.size++
			return true
		}
		queue = append(queue, cur.left, cur.right)
	}
	return false
}
func (t *Tree) IsMember(v int) bool {
	queue := []*treeNode{}
	if t.root != nil {
		queue = append(queue, t.root)
	}
	for i := 0; i < len(queue); i++ {
		n := queue[i]
		if n.data == v {
			return true
		}
		if n.left != nil {
			queue = append(queue, n.left)
		}
		if n.right != nil {
			queue = append(queue, n.right)
		}
	}
	return false
}
func (t *Tree) IsComplete() bool {
	queue := []*treeNode{t.root}
	gap := false
	for i := 0; i < len(queue); i++ {
		n := queue[i]
		if n == nil {
			gap = true
			continue
		}
		if gap {
			return false
		}
		queue = append(queue, n.left, n.right)
	}
	return true
}
func (t *Tree) Clear()      { *t = Tree{} }
func (t *Tree) Len() int    { return t.size }
func (t *Tree) Empty() bool { return t.root == nil }
func (t *Tree) Height() int {
	n := t.size
	h := 0
	for n > 0 {
		h++
		n >>= 1
	}
	return h
}
func (t *Tree) Items() []int {
	out := make([]int, 0, t.size)
	queue := []*treeNode{}
	if t.root != nil {
		queue = append(queue, t.root)
	}
	for i := 0; i < len(queue); i++ {
		n := queue[i]
		out = append(out, n.data)
		if n.left != nil {
			queue = append(queue, n.left)
		}
		if n.right != nil {
			queue = append(queue, n.right)
		}
	}
	return out
}
func (t *Tree) String() string {
	var sb strings.Builder
	for _, v := range t.Items() {
		sb.WriteString(" ")
		sb.WriteString(strconv.Itoa(v))
	}
	return sb.String()
}
func (t *Tree) Structure() string {
	var sb strings.Builder
	type item struct {
		node *treeNode
		path string
	}
	queue := []item{}
	if t.root != nil {
		queue = append(queue, item{t.root, "root"})
	}
	for i := 0; i < len(queue); i++ {
		it := queue[i]
		fmt.Fprintf(&sb, "%s = %d\n", it.path, it.node.data)
		if it.node.left != nil {
			queue = append(queue, item{it.node.left, it.path + "->left"})
		}
		if it.node.right != nil {
			queue = append(queue, item{it.node.right, it.path + "->right"})
		}
	}
	return sb.String()
}
func (t *Tree) levelOrder() []int { return t.Items() }
func (t *Tree) Clone() *Tree {
	c := &Tree{}
	for _, v := range t.Items() {
		c.Insert(v)
	}
	return c
}
func (t *Tree) replace(values []int) {
	fresh := Tree{}
	for _, v := range values {
		fresh.Insert(v)
	}
	*t = fresh
}
func (t *Tree) SerializeBinary(w io.Writer) error {
	if err := writeTag(w, "CTRE"); err != nil {
		return err
	}
	if err := writeSize(w, t.size); err != nil {
		return err
	}
	for _, v := range t.levelOrder() {
		if err := writeInt(w, v); err != nil {
			return err
		}
	}
	return nil
}

func (t *Tree) DeserializeBinary(r io.Reader) error {
	if err := readTag(r, "CTRE"); err != nil {
		return err
	}
	n, err := readSize(r)
	if err != nil {
		return err
	}
	values := make([]int, 0, n)
	for i := 0; i < n; i++ {
		v, err := readInt(r)
		if err != nil {
			return err
		}
		values = append(values, v)
	}
	t.replace(values)
	return nil
}

func (t *Tree) SerializeText(w io.Writer) error {
	if err := writeTextHeader(w, "CompleteTree", t.size); err != nil {
		return err
	}
	for _, v := range t.levelOrder() {
		if err := writeTextInt(w, v); err != nil {
			return err
		}
	}
	return nil
}

func (t *Tree) DeserializeText(r *bufio.Reader) error {
	n, err := readTextHeader(r, "CompleteTree")
	if err != nil {
		return err
	}
	values := make([]int, 0, n)
	for i := 0; i < n; i++ {
		v, err := readTextInt(r)
		if err != nil {
			return err
		}
		values = append(values, v)
	}
	t.replace(values)
	return nil
}
