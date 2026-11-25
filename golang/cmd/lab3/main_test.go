package main

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestCLI(t *testing.T) {
	dir := t.TempDir()
	require.NoError(t, run([]string{"--demo", dir}))
	for _, kind := range []string{"Array", "OneLinked", "Linked", "Stack", "Queue", "OpenHash", "LinearHash", "Tree"} {
		t.Run(kind, func(t *testing.T) {
			input := filepath.Join(dir, kind+".txt")
			binary := filepath.Join(dir, "converted.bin")
			output := filepath.Join(dir, "restored.txt")
			require.NoError(t, run([]string{"--convert", kind, "text", input, "binary", binary}))
			require.NoError(t, run([]string{"--convert", kind, "binary", binary, "text", output}))
			want, err := os.ReadFile(input)
			require.NoError(t, err)
			got, err := os.ReadFile(output)
			require.NoError(t, err)
			if kind != "OpenHash" && kind != "LinearHash" {
				assert.Equal(t, want, got)
			}
			// Ошибочный вход не должен повреждать уже существующий результат.
			bad := filepath.Join(dir, "bad.bin")
			raw, err := os.ReadFile(binary)
			require.NoError(t, err)
			for _, data := range [][]byte{raw[:5], append(append([]byte{}, raw...), 'X')} {
				require.NoError(t, os.WriteFile(bad, data, 0600))
				assert.Error(t, run([]string{"--convert", kind, "binary", bad, "text", output}))
				after, err := os.ReadFile(output)
				require.NoError(t, err)
				assert.Equal(t, got, after)
			}
		})
	}
	for _, args := range [][]string{
		{"--wrong"},
		{"--convert", "Unknown", "text", "in", "binary", "out"},
		{"--convert", "Array", "invalid", "in", "binary", "out"},
		{"--convert", "Array", "text", filepath.Join(dir, "missing"), "binary", "out"},
	} {
		assert.Error(t, run(args))
	}
	require.NoError(t, run([]string{"--help"}))
}
