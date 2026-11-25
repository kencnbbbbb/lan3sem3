# Лабораторная работа 3 — Беляков Максим Александрович, АБ-522

В одном проекте находятся **C++17 и Go**, реализации, тесты, бенчмарки,
сериализация, UML и фактические результаты проверки 25 ноября 2026 г.

Вариант дерева из личной ЛР1 — **3: Complete Binary Tree**. Вставка по уровням
слева направо, повторы сохраняются, есть проверка полноты. Хеширование из ЛР2 —
**вариант 5**: линейное пробирование и двойное хеширование DJB2/FNV-1a.
Теория отчёта соответствует нечётному варианту: российские стандарты качества ПО.

## Где код и тесты

- `str/*.h`, `str/*.cpp`, `main.cpp` — C++ и командная программа.
- `tests/gtest/` — Google Test; `tests/boost/` — Boost.Test; `tests/catch2/` — Catch2.
- `golang/ds/*.go` — Go; рядом находятся `*_test.go` с testing/testify.
- `golang/cmd/lab3/` — CLI Go и отдельный тест с 8 подтестами.
- `benchmarks/bench.cpp`, `golang/ds/bench_test.go` — бенчмарки обоих языков.
- `scripts/` — генерация покрытия, линтеры и проверка C++ ↔ Go.
- [Готовый отчёт DOCX](docs/REPORT.docx) и [PDF](docs/REPORT.pdf) — отчёт по предоставленному образцу.
- `reports/index.html` — навигация по фактическим результатам.
- `docs/uml_*.svg`, `docs/uml_*.mmd` — обзорные диаграммы и полные описания классов.

## Требования

CMake 3.20+, Clang с C++17, совместимые `llvm-cov` и `llvm-profdata`, Python 3.9+,
Go 1.22+ и make. Для C++-линтера:

```sh
python3 -m pip install cpplint==2.0.2
```

При первой сборке CMake скачивает закреплённые Google Test 1.17.0,
Boost.Test 1.88.0, Catch2 3.8.1 и Google Benchmark 1.9.4.
Для этого нужен интернет. Сторонние библиотеки C++ в архив не включены.
Go-зависимости находятся в `golang/vendor/` и работают без сети.

## Сборка и проверка из корня проекта

```sh
make all           # обе библиотеки C++ для тестов/бенчмарков и программы C++
make test          # все три набора тестов C++
make coverage      # три HTML-отчёта C++, проверка порога 85%
make test-go       # библиотека и CLI Go
make coverage-go   # HTML Go и порог 85% по каждому файлу
make lint          # Clang Static Analyzer, cpplint, go vet, gofmt
make demo          # файлы C++ и Go для всех структур
make verify        # все проверки и 1450 запусков межъязыкового сравнения
make bench         # три повтора каждого бенчмарка C++, затем Go
```

В Linux LLVM-инструменты должны соответствовать версии Clang; на macOS
они находятся через `xcrun` из Xcode Command Line Tools.
Команды без make:

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DLAB3_COVERAGE=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
python3 scripts/coverage.py --build build
cd golang
go test -mod=vendor ./... -count=1
go build -mod=vendor -o ../build/lab3-go ./cmd/lab3
cd ..
python3 scripts/cross_language_test.py --cpp build/lab3 --go build/lab3-go
```

На Windows можно использовать WSL и Linux-инструменты. Подготовленный код
фактически проверен на macOS arm64; Windows и Linux отдельно не проверялись.

## Контрольный пример дерева

```sh
./build/lab3 --convert Tree text examples/lab1_tree.txt binary build/tree.bin
./build/lab3-go --convert Tree binary build/tree.bin text build/tree.txt
cat build/tree.txt
```

Ожидается точное восстановление:

```text
CompleteTree 6
10
5
20
3
7
15
```

Типы CLI: `Array`, `OneLinked`, `Linked`, `Stack`, `Queue`, `OpenHash`,
`LinearHash`, `Tree`. `OpenHash` и `LinearHash` — два режима одного класса.
Форматы: `text`, `binary`. Оба языка используют совместимый little-endian формат;
строки текста заключаются в кавычки. Стек сохраняется от основания к вершине,
дерево — по уровням, хеш-таблица — как множество значений.

## Фактически получено

- Google Test: 132 теста из 17 наборов; строки 1087/1090 = 99,72%.
- Boost.Test: 51 тестовый случай, 12283 проверки; строки 1084/1090 = 99,45%.
- Catch2: 18 тестовых случаев, 12283 проверки; строки 1084/1090 = 99,45%.
- Go ds: 34 тестовые функции и 61 подтест; 774/780 операторов = 99,23%.
- Go CLI: 1 тест и 8 подтестов; 74/87 операторов = 85,06%.
- Go целиком: 848/867 операторов = 97,81%.
- Межъязыковое сравнение: 1450 успешных проверок процессов.
- Clang Static Analyzer, настроенные правила cpplint, go vet и gofmt: PASS.

Метрики C++ охватывают `str/`, Go — библиотеку и CLI. Это разные знаменатели и
разные виды метрик: строки C++ и операторы Go. Число проверок не равно числу
тестовых случаев. В таблицах бенчмарков приведены медианы трёх повторов;
у C++ используется `cpu_time`, у Go — `ns/op`, пересчитанные в микросекунды.
Сравнение времени зависит от среды, сборки и подготовки данных.

Личные исходные проекты не изменялись. Происхождение кода указано в SOURCE.md.

Исходники размещены в репозитории: https://github.com/kencnbbbbb/lan3sem3

Сводка актуальных проверок — `reports/index.html`. Дополнительные файлы прежних
запусков, уже находившиеся в репозитории, сохранены; их даты и метрики могут
отличаться от проверки 25 ноября 2026 г.
