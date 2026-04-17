// #include <benchmark/benchmark.h>
#include "dynamic_array.h"
#include <iostream>
#include <vector>
using namespace std;

// struct S { 
//     int data[16];
//     std::string name;
// };

// static void BM_Test_vector_push_back(benchmark::State& state) {
//     for (auto _ : state) {
//         std::vector<S> arr;
//         for (int i = 0; i < 10; i++) arr.push_back(S({12, 12, 12}, "Anshu"));
//     }
// }

// static void BM_Test_dynamic_array_push(benchmark::State& state) {
//     for (auto _ : state) {
//         DynamicArray<S> arr;
//         for (int i = 0; i < 10; i++) arr.push(S({ 12, 12, 12 }, "Anshu"));
//     }
// }

//BENCHMARK(BM_Test_vector_push_back)->Range(1 << 10, 1 << 20);
//BENCHMARK(BM_Test_dynamic_array_push)->Range(1 << 10, 1 << 20);
//
//BENCHMARK_MAIN();

int main() {
    DynamicArray<int> arr;
    arr.push(12);
    arr.push(13);
    arr.push(11);
    arr.push(10);

    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }

    return 0;
}