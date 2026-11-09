#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include "../str/array.h"
#include "../str/linked_list.h"
#include "../str/one_linked_list.h"
#include "../str/open_hash.h"
#include "../str/queue.h"
#include "../str/stack.h"
#include "../str/tree.h"

namespace {

vector<string> make_keys(size_t n) {
    vector<string> keys;
    keys.reserve(n);
    for (size_t i = 0; i < n; i++) keys.push_back("key" + to_string(i));
    return keys;
}

}

static void BM_ArrayPushBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        Array a;
        for (const auto& k : keys) a.push_back(k);
        benchmark::DoNotOptimize(a);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_ArrayPushBack)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

static void BM_ArrayInsertFront(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        Array a;
        for (const auto& k : keys) a.insert(0, k);
        benchmark::DoNotOptimize(a);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_ArrayInsertFront)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

static void BM_OneLinkedPushBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        OneLinked l;
        for (const auto& k : keys) l.push_back(k);
        benchmark::DoNotOptimize(l);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_OneLinkedPushBack)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_OneLinkedPopBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        state.PauseTiming();
        OneLinked l;
        for (const auto& k : keys) l.push_back(k);
        state.ResumeTiming();
        while (!l.empty()) benchmark::DoNotOptimize(l.pop_back());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_OneLinkedPopBack)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

static void BM_LinkedPushBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        Linked l;
        for (const auto& k : keys) l.push_back(k);
        benchmark::DoNotOptimize(l);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_LinkedPushBack)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_LinkedPopBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        state.PauseTiming();
        Linked l;
        for (const auto& k : keys) l.push_back(k);
        state.ResumeTiming();
        while (!l.empty()) benchmark::DoNotOptimize(l.pop_back());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_LinkedPopBack)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_StackPushPop(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        Stack s;
        for (const auto& k : keys) s.push(k);
        while (!s.empty()) benchmark::DoNotOptimize(s.pop());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_StackPushPop)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_QueuePushPop(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        Queue q;
        for (const auto& k : keys) q.push(k);
        while (!q.empty()) benchmark::DoNotOptimize(q.pop());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_QueuePushPop)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_OpenHashInsert(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        OpenHash h;
        for (const auto& k : keys) h.insert(k);
        benchmark::DoNotOptimize(h);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_OpenHashInsert)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

static void BM_OpenHashContains(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    OpenHash h;
    for (const auto& k : keys) h.insert(k);
    for (auto _ : state) {
        for (const auto& k : keys) benchmark::DoNotOptimize(h.contains(k));
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_OpenHashContains)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

static void BM_TreeInsert(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        Tree t;
        for (int i = 0; i < n; i++) t.insert((i * 7919) % (n * 10));
        benchmark::DoNotOptimize(t);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_TreeInsert)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_TreeMember(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    Tree t;
    for (int i = 0; i < n; i++) t.insert(i);
    for (auto _ : state) {
        for (int i = 0; i < n; i++) benchmark::DoNotOptimize(t.is_member(i));
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_TreeMember)->RangeMultiplier(10)->Range(100, 1000)->Complexity();

static void BM_LinearHashInsert(benchmark::State& state) {
    const auto keys = make_keys(static_cast<size_t>(state.range(0)));
    for (auto _ : state) {
        OpenHash h(16, ProbeMode::Linear);
        for (const auto& key : keys) h.insert(key);
        benchmark::DoNotOptimize(h);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_LinearHashInsert)->RangeMultiplier(10)->Range(100, 10000)->Complexity();

BENCHMARK_MAIN();
