#include <benchmark/benchmark.h>
#include "dynamic_array.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Test Cases
// 1. Primitive Types
// 1.A. int
static void BM_Test_vector_push_back_int(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> arr;
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();

        int n = state.range(0);
        for (int i = 0; i < n; i++) arr.push_back(10000);
    }
}

static void BM_Test_dynamic_array_push_int(benchmark::State& state) {
    for (auto _ : state) {
        DynamicArray<int> arr;
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();

        int n = state.range(0);
        for (int i = 0; i < n; i++) arr.push(1000);
    }
}

BENCHMARK(BM_Test_vector_push_back_int)->Range(1 << 10, 1 << 20);
BENCHMARK(BM_Test_dynamic_array_push_int)->Range(1 << 10, 1 << 20);

BENCHMARK_MAIN();

// #include <iostream>
// #include <vector>
// using namespace std;


// class S {
// private:
//     int x;
//     string y;
// public:
//     S() = default;
//     ~S() noexcept(false) {
//         throw;
//     }
//     S(const S& other) = delete;
// };

// int main() {
//     vector<S> arr;

//     arr.push_back(S());
//     arr.push_back(S());
//     arr.push_back(S());
//     arr.push_back(S());
//     arr.push_back(S());
//     arr.push_back(S());

//     return 0;
// }