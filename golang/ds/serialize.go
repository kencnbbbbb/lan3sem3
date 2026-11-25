package ds

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
	"strconv"
	"strings"
	"unicode"
)

const MaxItems = 1 << 24

type Serializer interface {
	SerializeBinary(w io.Writer) error
	DeserializeBinary(r io.Reader) error
	SerializeText(w io.Writer) error
	DeserializeText(r *bufio.Reader) error
}

func writeTag(w io.Writer, tag string) error {
	n, err := io.WriteString(w, tag)
	if err == nil && n != len(tag) {
		return io.ErrShortWrite
	}
	return err
}

func readTag(r io.Reader, tag string) error {
	buf := make([]byte, 4)
	if _, err := io.ReadFull(r, buf); err != nil {
		return fmt.Errorf("%w: тег: %w", ErrFormat, err)
	}
	if string(buf) != tag {
		return fmt.Errorf("%w: ожидался тег %s, получен %q", ErrFormat, tag, buf)
	}
	return nil
}

func writeSize(w io.Writer, n int) error {
	return binary.Write(w, binary.LittleEndian, uint64(n))
}

func readSize(r io.Reader) (int, error) {
	var n uint64
	if err := binary.Read(r, binary.LittleEndian, &n); err != nil {
		return 0, fmt.Errorf("%w: размер: %w", ErrFormat, err)
	}
	if n > MaxItems {
		return 0, fmt.Errorf("%w: слишком большой размер %d", ErrFormat, n)
	}
	return int(n), nil
}

func writeString(w io.Writer, s string) error {
	if err := writeSize(w, len(s)); err != nil {
		return err
	}
	written, err := io.WriteString(w, s)
	if err == nil && written != len(s) {
		return io.ErrShortWrite
	}
	return err
}

func readString(r io.Reader) (string, error) {
	n, err := readSize(r)
	if err != nil {
		return "", err
	}
	buf := make([]byte, n)
	if _, err := io.ReadFull(r, buf); err != nil {
		return "", fmt.Errorf("%w: строка: %w", ErrFormat, err)
	}
	return string(buf), nil
}

func writeInt(w io.Writer, v int) error {
	if v < math.MinInt32 || v > math.MaxInt32 {
		return fmt.Errorf("%w: число %d не помещается в int32", ErrFormat, v)
	}
	return binary.Write(w, binary.LittleEndian, int32(v))
}

func readInt(r io.Reader) (int, error) {
	var v int32
	if err := binary.Read(r, binary.LittleEndian, &v); err != nil {
		return 0, fmt.Errorf("%w: число: %w", ErrFormat, err)
	}
	return int(v), nil
}

func readToken(r *bufio.Reader) (string, error) {
	var sb strings.Builder
	for {
		c, _, err := r.ReadRune()
		if err != nil {
			if sb.Len() > 0 && errors.Is(err, io.EOF) {
				return sb.String(), nil
			}
			return "", fmt.Errorf("%w: ожидалось слово: %w", ErrFormat, err)
		}
		if unicode.IsSpace(c) {
			if sb.Len() > 0 {
				return sb.String(), nil
			}
			continue
		}
		sb.WriteRune(c)
	}
}

func writeTextHeader(w io.Writer, tag string, n int) error {
	_, err := fmt.Fprintf(w, "%s %d\n", tag, n)
	return err
}

func readTextHeader(r *bufio.Reader, tag string) (int, error) {
	got, err := readToken(r)
	if err != nil {
		return 0, err
	}
	if got != tag {
		return 0, fmt.Errorf("%w: ожидался заголовок %s, получен %q", ErrFormat, tag, got)
	}
	n, err := readTextInt(r)
	if err != nil {
		return 0, err
	}
	if n < 0 || n > MaxItems {
		return 0, fmt.Errorf("%w: недопустимый размер %d", ErrFormat, n)
	}
	return n, nil
}

func readTextInt(r *bufio.Reader) (int, error) {
	tok, err := readToken(r)
	if err != nil {
		return 0, err
	}
	v, err := strconv.ParseInt(tok, 10, 32)
	if err != nil {
		return 0, fmt.Errorf("%w: не число %q", ErrFormat, tok)
	}
	return int(v), nil
}

func PeekTextTag(r *bufio.Reader) (string, error) {
	buf, _ := r.Peek(64)
	i := 0
	for i < len(buf) && unicode.IsSpace(rune(buf[i])) {
		i++
	}
	j := i
	for j < len(buf) && !unicode.IsSpace(rune(buf[j])) {
		j++
	}
	if i == j {
		return "", fmt.Errorf("%w: ожидался тег структуры", ErrFormat)
	}
	return string(buf[i:j]), nil
}

func PeekBinaryTag(r *bufio.Reader) (string, error) {
	buf, err := r.Peek(4)
	if err != nil {
		return "", fmt.Errorf("%w: тег структуры: %w", ErrFormat, err)
	}
	return string(buf), nil
}

func WriteTag(w io.Writer, tag string) error { return writeTag(w, tag) }

func ReadTag(r io.Reader, tag string) error { return readTag(r, tag) }

func WriteSize(w io.Writer, n int) error { return writeSize(w, n) }

func ReadSize(r io.Reader) (int, error) { return readSize(r) }

func WriteString(w io.Writer, s string) error { return writeString(w, s) }

func ReadString(r io.Reader) (string, error) { return readString(r) }

func WriteTextHeader(w io.Writer, tag string, n int) error { return writeTextHeader(w, tag, n) }

func ReadTextHeader(r *bufio.Reader, tag string) (int, error) { return readTextHeader(r, tag) }

func WriteTextString(w io.Writer, s string) error { return writeTextString(w, s) }

func ReadTextString(r *bufio.Reader) (string, error) { return readTextString(r) }

func writeTextString(w io.Writer, s string) error {
	var sb strings.Builder
	sb.WriteByte('"')
	for i := 0; i < len(s); i++ {
		if s[i] == '"' || s[i] == '\\' {
			sb.WriteByte('\\')
		}
		sb.WriteByte(s[i])
	}
	sb.WriteString("\"\n")
	_, err := io.WriteString(w, sb.String())
	return err
}

func readTextString(r *bufio.Reader) (string, error) {
	for {
		b, err := r.ReadByte()
		if err != nil {
			return "", fmt.Errorf("%w: ожидалась строка: %w", ErrFormat, err)
		}
		if b == '"' {
			break
		}
		if !unicode.IsSpace(rune(b)) {
			return "", fmt.Errorf("%w: строка должна начинаться с кавычки", ErrFormat)
		}
	}
	var sb strings.Builder
	for {
		b, err := r.ReadByte()
		if err != nil {
			return "", fmt.Errorf("%w: незакрытая строка: %w", ErrFormat, err)
		}
		switch b {
		case '"':
			return sb.String(), nil
		case '\\':
			next, err := r.ReadByte()
			if err != nil {
				return "", fmt.Errorf("%w: обрыв после \\: %w", ErrFormat, err)
			}
			sb.WriteByte(next)
		default:
			sb.WriteByte(b)
		}
	}
}

func writeTextInt(w io.Writer, v int) error {
	_, err := fmt.Fprintf(w, "%d\n", v)
	return err
}

func serializeStringsBinary(w io.Writer, tag string, items []string) error {
	if err := writeTag(w, tag); err != nil {
		return err
	}
	if err := writeSize(w, len(items)); err != nil {
		return err
	}
	for _, s := range items {
		if err := writeString(w, s); err != nil {
			return err
		}
	}
	return nil
}

func deserializeStringsBinary(r io.Reader, tag string) ([]string, error) {
	if err := readTag(r, tag); err != nil {
		return nil, err
	}
	n, err := readSize(r)
	if err != nil {
		return nil, err
	}
	items := make([]string, 0, n)
	for i := 0; i < n; i++ {
		s, err := readString(r)
		if err != nil {
			return nil, err
		}
		items = append(items, s)
	}
	return items, nil
}

func serializeStringsText(w io.Writer, tag string, items []string) error {
	if err := writeTextHeader(w, tag, len(items)); err != nil {
		return err
	}
	for _, s := range items {
		if err := writeTextString(w, s); err != nil {
			return err
		}
	}
	return nil
}

func deserializeStringsText(r *bufio.Reader, tag string) ([]string, error) {
	n, err := readTextHeader(r, tag)
	if err != nil {
		return nil, err
	}
	items := make([]string, 0, n)
	for i := 0; i < n; i++ {
		s, err := readTextString(r)
		if err != nil {
			return nil, err
		}
		items = append(items, s)
	}
	return items, nil
}
