#include <benchmark/benchmark.h>
#include "dynamic_array.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct S { 
    string name;
    int data;
};

static void BM_Test_vector_push_back(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<S> arr;
        for (int i = 0; i < 10; i++) arr.push_back(S());
    }
}

static void BM_Test_dynamic_array_push(benchmark::State& state) {
    for (auto _ : state) {
        DynamicArray<S> arr;
        for (int i = 0; i < 10; i++) arr.push(S());
    }
}

BENCHMARK(BM_Test_vector_push_back)->Range(1 << 10, 1 << 20);
BENCHMARK(BM_Test_dynamic_array_push)->Range(1 << 10, 1 << 20);

BENCHMARK_MAIN();