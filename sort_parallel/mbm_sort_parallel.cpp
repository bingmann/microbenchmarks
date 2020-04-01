/*******************************************************************************
 * mbm_sort_parallel.cpp
 *
 * Microbenchmark parallel sorting algorithms
 *
 * Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the MIT License in the LICENSE file.
 ******************************************************************************/

#include <microbenchmarking.hpp>

#include <tlx/die.hpp>
#include <tlx/string/contains.hpp>

#include <algorithm>
#include <iostream>
#include <random>

/******************************************************************************/
// Settings

//! starting number of items to insert
const size_t min_size = 1024 * 1024;

//! maximum number of items to insert
const size_t max_size = 512 * 1024 * 1024;

/******************************************************************************/

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
// Parallel Sorters

#include "ips4o/ips4o.hpp"

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

#include <tbb/parallel_sort.h>

class TBBParallelSort : public SortBenchmark {
public:
    TBBParallelSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "tbb::parallel_sort";
    }
    void run() {
        tbb::parallel_sort(vec_.begin(), vec_.end(), cmp_);
    }
};

#include "extra/msd_parallel_radixsort.hpp"

uint8_t radix_extract_key(const MyStruct& s, size_t depth)
{
    return tlx::parallel_radixsort_detail::get_key<uint32_t, uint8_t>(s.a, depth);
}

class ParallelMSDRadixSort : public SortBenchmark {
public:
    ParallelMSDRadixSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "parallel_msd_radixsort";
    }
    void run() {
        tlx::parallel_radixsort_detail::radix_sort<
            std::vector<MyStruct>::iterator, radix_extract_key>(
            vec_.begin(), vec_.end(), sizeof(uint32_t));
    }
};

#include "extra/lsd_radix_sort_prefix.hpp"

class ParallelLSDRadixSort : public SortBenchmark {
public:
    ParallelLSDRadixSort(size_t size, size_t rep) : SortBenchmark(size, rep) {
    }
    const char* name() const final {
        return "parallel_lsd_radixsort";
    }
    void run() {
        auto getter = [](const MyStruct& s) { return s.a; };
        rdx::radix_sort_prefix_par(vec_.begin(), vec_.end(), getter);
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
    for (size_t size = min_size; size <= max_size; size = 2 * size) {
        size_t f = (8 * 1024 * 1024) / size;
        for (size_t rep = 0; rep < std::max<size_t>(10, 100 * f); ++rep) {
            // MBM_ALGORITHM is defined from cmake to select algorithm
            test_size<MBM_ALGORITHM>(size, rep);
        }
    }

    return 0;
}

/******************************************************************************/
