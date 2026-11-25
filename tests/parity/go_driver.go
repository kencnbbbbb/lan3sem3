// Test-only adapter; built from the golang module, excluded from production coverage.
package main

import (
	"bufio"
	"encoding/hex"
	"fmt"
	"io"
	"laba3/ds"
	"os"
	"strconv"
	"strings"
)

func boolean(value bool) string {
	if value {
		return "1"
	}
	return "0"
}
func encode(value string) string {
	if value == "" {
		return "-"
	}
	return hex.EncodeToString([]byte(value))
}
func decode(value string) string {
	if value == "-" {
		return ""
	}
	data, err := hex.DecodeString(value)
	if err != nil {
		panic(err)
	}
	return string(data)
}

type list interface {
	PushBack(string)
	PushFront(string)
	PushAfter(string, string) error
	PushBefore(string, string) error
	PopBack() (string, error)
	PopFront() (string, error)
	PopAfter(string) (string, error)
	PopBefore(string) (string, error)
	Find(string) int
	DelByValue(string) bool
	StringReverse() string
	Clear()
}

func listOperation(a list, op, v, w string) (string, error) {
	switch op {
	case "pushback":
		a.PushBack(v)
	case "pushfront":
		a.PushFront(v)
	case "after":
		return "", a.PushAfter(v, w)
	case "before":
		return "", a.PushBefore(v, w)
	case "popback":
		return a.PopBack()
	case "popfront":
		return a.PopFront()
	case "popafter":
		return a.PopAfter(v)
	case "popbefore":
		return a.PopBefore(v)
	case "find":
		return strconv.Itoa(a.Find(v)), nil
	case "del":
		return boolean(a.DelByValue(v)), nil
	case "reverse":
		return a.StringReverse(), nil
	case "clear":
		a.Clear()
	default:
		return "", fmt.Errorf("operation")
	}
	return "", nil
}
func operate(obj ds.Serializer, op string, i int, v, w string) (string, error) {
	if op == "len" {
		return strconv.Itoa(obj.(interface{ Len() int }).Len()), nil
	}
	if op == "empty" {
		return boolean(obj.(interface{ Empty() bool }).Empty()), nil
	}
	if op == "clone" {
		switch a := obj.(type) {
		case *ds.Array:
			c := a.Clone()
			err := c.PushBack(v)
			return strconv.Itoa(c.Len()), err
		case *ds.OneLinked:
			c := a.Clone()
			c.PushBack(v)
			return strconv.Itoa(c.Len()), nil
		case *ds.Linked:
			c := a.Clone()
			c.PushBack(v)
			return strconv.Itoa(c.Len()), nil
		case *ds.Stack:
			c := a.Clone()
			c.Push(v)
			return strconv.Itoa(c.Len()), nil
		case *ds.Queue:
			c := a.Clone()
			c.Push(v)
			return strconv.Itoa(c.Len()), nil
		case *ds.OpenHash:
			c := a.Clone()
			_, err := c.Insert(v)
			return strconv.Itoa(c.Len()), err
		case *ds.Tree:
			c := a.Clone()
			c.Insert(i)
			return strconv.Itoa(c.Len()), nil
		}
	}
	switch a := obj.(type) {
	case *ds.Array:
		switch op {
		case "push":
			return "", a.PushBack(v)
		case "insert":
			return "", a.Insert(i, v)
		case "get":
			return a.Get(i)
		case "set":
			return "", a.Set(i, v)
		case "erase":
			return "", a.Erase(i)
		case "cap":
			return strconv.Itoa(a.Cap()), nil
		case "clear":
			a.Clear()
			return "", nil
		}
	case *ds.OneLinked:
		return listOperation(a, op, v, w)
	case *ds.Linked:
		return listOperation(a, op, v, w)
	case *ds.Stack:
		switch op {
		case "push":
			a.Push(v)
			return "", nil
		case "pop":
			return a.Pop()
		case "peek":
			return a.Top()
		case "clear":
			a.Clear()
			return "", nil
		}
	case *ds.Queue:
		switch op {
		case "push":
			a.Push(v)
			return "", nil
		case "pop":
			return a.Pop()
		case "peek":
			return a.Front()
		case "clear":
			a.Clear()
			return "", nil
		}
	case *ds.OpenHash:
		switch op {
		case "insert":
			added, err := a.Insert(v)
			return boolean(added), err
		case "del":
			return boolean(a.Erase(v)), nil
		case "contains", "find":
			return boolean(a.Contains(v)), nil
		case "clear":
			a.Clear()
			return "", nil
		}
	case *ds.Tree:
		switch op {
		case "insert":
			return boolean(a.Insert(i)), nil
		case "contains":
			return boolean(a.IsMember(i)), nil
		case "complete":
			return boolean(a.IsComplete()), nil
		case "height":
			return strconv.Itoa(a.Height()), nil
		case "structure":
			return a.Structure(), nil
		case "clear":
			a.Clear()
			return "", nil
		}
	}
	return "", fmt.Errorf("operation")
}
func main() {
	if len(os.Args) != 2 {
		os.Exit(2)
	}
	var obj ds.Serializer
	switch os.Args[1] {
	case "Array":
		obj, _ = ds.NewArray(0)
	case "OneLinked":
		obj = &ds.OneLinked{}
	case "Linked":
		obj = &ds.Linked{}
	case "Stack":
		obj = &ds.Stack{}
	case "Queue":
		obj = &ds.Queue{}
	case "OpenHash":
		obj, _ = ds.NewOpenHash(0)
	case "LinearHash":
		obj, _ = ds.NewLinearHash(0)
	case "Tree":
		obj = &ds.Tree{}
	default:
		os.Exit(2)
	}
	input := bufio.NewReader(os.Stdin)
	output := bufio.NewWriter(os.Stdout)
	defer output.Flush()
	for {
		var op, v, w string
		var index int
		_, err := fmt.Fscan(input, &op, &index, &v, &w)
		if err == io.EOF {
			return
		}
		if err != nil {
			panic(err)
		}
		result, err := operate(obj, op, index, decode(v), decode(w))
		status := "OK"
		if err != nil {
			status, result = "ERROR", ""
		}
		var state strings.Builder
		if err := obj.SerializeText(&state); err != nil {
			panic(err)
		}
		fmt.Fprintf(output, "%s\t%s\t%s\n", status, encode(result), encode(state.String()))
	}
}
