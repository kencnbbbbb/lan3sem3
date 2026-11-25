package main

import (
	"bufio"
	"fmt"
	"io"
	"laba3/ds"
	"os"
	"path/filepath"
	"strings"
)

func create(kind string) (ds.Serializer, error) {
	switch kind {
	case "Array":
		return ds.NewArray(0)
	case "OneLinked":
		return &ds.OneLinked{}, nil
	case "Linked":
		return &ds.Linked{}, nil
	case "Stack":
		return &ds.Stack{}, nil
	case "Queue":
		return &ds.Queue{}, nil
	case "OpenHash":
		return ds.NewOpenHash(0)
	case "LinearHash":
		return ds.NewLinearHash(0)
	case "Tree":
		return &ds.Tree{}, nil
	}
	return nil, fmt.Errorf("неизвестный тип структуры")
}
func write(obj ds.Serializer, format, path string) error {
	if format != "text" && format != "binary" {
		return fmt.Errorf("формат: text или binary")
	}
	f, e := os.Create(path)
	if e != nil {
		return e
	}
	if format == "binary" {
		e = obj.SerializeBinary(f)
	} else {
		e = obj.SerializeText(f)
	}
	closeErr := f.Close()
	if e != nil {
		return e
	}
	return closeErr
}
func convert(kind, from, input, to, output string) error {
	if (from != "text" && from != "binary") || (to != "text" && to != "binary") {
		return fmt.Errorf("формат: text или binary")
	}
	obj, e := create(kind)
	if e != nil {
		return e
	}
	f, e := os.Open(input)
	if e != nil {
		return e
	}
	defer f.Close()
	r := bufio.NewReader(f)
	if from == "binary" {
		e = obj.DeserializeBinary(r)
	} else {
		e = obj.DeserializeText(r)
	}
	if e != nil {
		return e
	}
	rest, e := io.ReadAll(r)
	if e != nil {
		return e
	}
	if from == "text" {
		rest = []byte(strings.TrimSpace(string(rest)))
	}
	if len(rest) > 0 {
		return fmt.Errorf("лишние данные после структуры")
	}
	return write(obj, to, output)
}
func demo(dir string) error {
	if e := os.MkdirAll(dir, 0755); e != nil {
		return e
	}
	a, _ := ds.NewArray(0)
	one := &ds.OneLinked{}
	two := &ds.Linked{}
	stack := &ds.Stack{}
	queue := &ds.Queue{}
	hash, _ := ds.NewOpenHash(0)
	linear, _ := ds.NewLinearHash(0)
	tree := &ds.Tree{}
	for _, v := range []string{"alpha", "two words", "", "q\"uote", "Привет"} {
		if e := a.PushBack(v); e != nil {
			return e
		}
		one.PushBack(v)
		two.PushBack(v)
		stack.Push(v)
		queue.Push(v)
		if _, e := hash.Insert(v); e != nil {
			return e
		}
		if _, e := linear.Insert(v); e != nil {
			return e
		}
	}
	for _, v := range []int{5, 3, 8, 3, -7, 0, 11, 5} {
		tree.Insert(v)
	}
	objects := map[string]ds.Serializer{"Array": a, "OneLinked": one, "Linked": two, "Stack": stack, "Queue": queue, "OpenHash": hash, "LinearHash": linear, "Tree": tree}
	for name, obj := range objects {
		if e := write(obj, "text", filepath.Join(dir, name+".txt")); e != nil {
			return e
		}
		if e := write(obj, "binary", filepath.Join(dir, name+".bin")); e != nil {
			return e
		}
	}
	fmt.Println("Созданы текстовые и бинарные примеры для 8 структур")
	return nil
}
func run(args []string) error {
	if len(args) == 2 && args[0] == "--demo" {
		return demo(args[1])
	}
	if len(args) == 6 && args[0] == "--convert" {
		return convert(args[1], args[2], args[3], args[4], args[5])
	}
	if len(args) == 0 || (len(args) == 1 && args[0] == "--help") {
		fmt.Println("lab3 --demo папка\nlab3 --convert Тип text|binary вход text|binary выход")
		return nil
	}
	return fmt.Errorf("неверные параметры; используйте --help")
}
func main() {
	if e := run(os.Args[1:]); e != nil {
		fmt.Fprintln(os.Stderr, "Ошибка:", e)
		os.Exit(1)
	}
}
