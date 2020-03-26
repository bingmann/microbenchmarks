/*******************************************************************************
 * mbm_sort.cpp
 *
 * Microbenchmark sorting algorithms
 *
 * Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the MIT License in the LICENSE file.
 ******************************************************************************/

#include <microbenchmarking.hpp>

#include <tlx/die.hpp>

#include <algorithm>
#include <iostream>
#include <random>

struct MyStruct {
    uint32_t a, b;

    explicit MyStruct(uint32_t x = 0) : a(x), b(x * x) {
    }

    bool operator<(const MyStruct& other) const {
        return a < other.a;
    }

    friend std::ostream& operator<<(std::ostream& os, const MyStruct& s) {
        return os << '(' << s.a << ',' << s.b << ')';
    }
};

class SortBenchmark {
public:
    std::vector<MyStruct> vec_;
    std::less<MyStruct> cmp_;

    SortBenchmark(size_t size, size_t rep) {
        std::mt19937 rng(123456 + rep);
        std::uniform_int_distribution<uint32_t> distr;

        vec_.resize(size);
        for (unsigned int i = 0; i < size; ++i)
            vec_[i] = MyStruct(distr(rng));
    }

    void check() {
        die_unless(std::is_sorted(vec_.begin(), vec_.end(), cmp_));
    }

    virtual const char* name() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const SortBenchmark& b) {
        return os << "benchmark=" << b.name() << '\t'
                  << "size=" << b.vec_.size() << '\t';
    }
};

/******************************************************************************/
// Sequential Sorters

class StdSort : public SortBenchmark {
public:
    StdSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "std::sort";
    }
    void run() {
        std::sort(vec_.begin(), vec_.end(), cmp_);
    }
};

class StdStableSort : public SortBenchmark {
public:
    StdStableSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "std::stable_sort";
    }
    void run() {
        std::stable_sort(vec_.begin(), vec_.end(), cmp_);
    }
};

#include "ips4o/ips4o.hpp"

class IPS4oSort : public SortBenchmark {
public:
    IPS4oSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "ips4o::sort";
    }
    void run() {
        ips4o::sort(vec_.begin(), vec_.end(), cmp_);
    }
};

/******************************************************************************/
// Parallel Sorters

class IPS4oParallelSort : public SortBenchmark {
public:
    IPS4oParallelSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "ips4o::parallel_sort";
    }
    void run() {
        ips4o::parallel::sort(vec_.begin(), vec_.end(), cmp_);
    }
};

#include <tlx/sort/parallel_mergesort.hpp>

class MCSTLParallelMergesort : public SortBenchmark {
public:
    MCSTLParallelMergesort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "mcstl::parallel_sort";
    }
    void run() {
        tlx::parallel_mergesort(vec_.begin(), vec_.end(), cmp_);
    }
};

/******************************************************************************/

template <typename Benchmark>
void test_size(size_t size, size_t rep) {

    Microbenchmark mbm;
    mbm.enable_hw_cpu_cycles();
    mbm.enable_hw_instructions();
    mbm.enable_hw_ref_cpu_cycles();

    mbm.enable_hw_cache1(
        PerfCache::L1I, PerfCacheOp::Read, PerfCacheOpResult::Miss);
    mbm.enable_hw_cache2(
        PerfCache::L1D, PerfCacheOp::Read, PerfCacheOpResult::Miss);
    mbm.enable_hw_cache3(
        PerfCache::LL, PerfCacheOp::Read, PerfCacheOpResult::Miss);

    mbm.run_check_print(Benchmark(size, rep));
}

int main() {
    for (size_t size = 1024; size < 32 * 1024 * 1024; size = 2 * size) {
        size_t f = (8 * 1024 * 1024) / size;
        for (size_t rep = 0; rep < std::min<size_t>(1000, 100 * f); ++rep) {
            // MBM_ALGORITHM is defined from cmake to select algorithm
            test_size<MBM_ALGORITHM>(size, rep);

            // test_size<StdSort>(size, rep);
            // test_size<StdStableSort>(size, rep);
            // test_size<IPS4oSort>(size, rep);

            // test_size<IPS4oParallelSort>(size, rep);
            // test_size<MCSTLParallelMergesort>(size, rep);
        }
    }

    return 0;
}

/******************************************************************************/
