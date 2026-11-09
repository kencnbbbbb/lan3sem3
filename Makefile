CMAKE ?= cmake
PYTHON ?= python3
CXX = clang++
BUILD ?= build
.PHONY: all configure test test-go coverage coverage-go lint bench demo verify clean
all: configure
	$(CMAKE) --build $(BUILD) --parallel 4
configure:
	$(CMAKE) -S . -B $(BUILD) -DCMAKE_CXX_COMPILER=$(CXX) -DLAB3_COVERAGE=ON
test: all
	ctest --test-dir $(BUILD) --output-on-failure
test-go:
	cd golang && go test ./... -count=1
coverage: all
	$(PYTHON) scripts/coverage.py --build $(BUILD)
coverage-go:
	mkdir -p reports/go
	cd golang && go test ./ds -covermode=atomic -coverprofile=../reports/go/coverage.out -count=1
	cd golang && go tool cover -html=../reports/go/coverage.out -o ../reports/go/index.html
	$(PYTHON) scripts/check_go_coverage.py reports/go/coverage.out
lint:
	$(PYTHON) scripts/lint.py
bench: all
	./$(BUILD)/benchmarks --benchmark_min_time=0.05s --benchmark_repetitions=3 --benchmark_out=reports/cpp_benchmarks.json --benchmark_out_format=json
	cd golang && go test ./ds -run '^$$' -bench . -benchmem -benchtime=50ms -count=3
demo: all
	./$(BUILD)/lab3 --demo examples/cpp
	cd golang && go run ./cmd/lab3 --demo ../examples/go
verify: coverage coverage-go test-go lint
	cd golang && go build -o ../$(BUILD)/lab3-go ./cmd/lab3
	$(PYTHON) scripts/cross_language_test.py --cpp $(BUILD)/lab3 --go $(BUILD)/lab3-go
clean:
	rm -rf $(BUILD)
