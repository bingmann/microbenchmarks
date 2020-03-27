/*******************************************************************************
 * mbm_unordered_sets.cpp
 *
 * Microbenchmark insertion, find, and delete in unordered sets and maps.
 *
 * Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the MIT License in the LICENSE file.
 ******************************************************************************/

#include <microbenchmarking.hpp>

#include <tlx/die.hpp>
#include <tlx/unused.hpp>

#include <algorithm>
#include <iostream>
#include <random>

#include <unordered_map>
#include <unordered_set>

#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_map>
#include <sparsehash/sparse_hash_set>

#include <sparsepp/spp.h>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <robin_hood.h>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>

/******************************************************************************/
// Settings

//! starting number of items to insert
const size_t min_items = 125;

//! maximum number of items to insert
const size_t max_items = 1024000 * 16;

//! maximum number of items to insert
const size_t target_items = 1024000 * 16;

//! random seed
const int seed = 34234235;

/******************************************************************************/

class Benchmark {
public:
    Benchmark(size_t size, const char* container)
        : size_(size), container_(container) {
    }

    size_t size_;
    const char* container_;

    virtual const char* name() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Benchmark& b) {
        return os << "benchmark=" << b.name() << '\t'
                  << "container=" << b.container_ << '\t' << "size=" << b.size_
                  << '\t';
    }
};

//! adjust sentinel values
size_t adjust(size_t x) {
    return x < 2 ? 2 : x;
}

/******************************************************************************/
// Set Benchmarks

//! Test a generic set type with insertions
template <typename SetType>
class Test_Set_Insert : public Benchmark {
public:
    Test_Set_Insert(size_t size, const char* container)
        : Benchmark(size, container) {
    }

    const char* name() const final {
        return "set_insert";
    }

    void run() {
        SetType set;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(adjust(rng()));

        die_unless(static_cast<size_t>(set.size()) == size_);
    }
};

//! Test a generic set type with insert, find and delete sequences
template <typename SetType>
class Test_Set_InsertFindDelete : public Benchmark {
public:
    Test_Set_InsertFindDelete(size_t size, const char* container)
        : Benchmark(size, container) {
    }

    const char* name() const final {
        return "set_insert_find_delete";
    }

    void run() {
        SetType set;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(adjust(rng()));

        die_unequal(static_cast<size_t>(set.size()), size_);

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            set.find(adjust(rng()));

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            set.erase(set.find(adjust(rng())));

        die_unless(set.empty());
    }
};

//! Test a generic set type with insert, find and delete sequences
template <typename SetType>
class Test_Set_Find : public Benchmark {
public:
    SetType set;

    const char* name() const final {
        return "set_find";
    }

    Test_Set_Find(size_t size, const char* container)
        : Benchmark(size, container) {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(adjust(rng()));

        die_unless(static_cast<size_t>(set.size()) == size_);
    }

    void run() {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.find(adjust(rng()));
    }
};

/*----------------------------------------------------------------------------*/
// Set Adapters

class MyGoogleSparseHashSet : public google::sparse_hash_set<size_t> {
public:
    MyGoogleSparseHashSet() : google::sparse_hash_set<size_t>() {
        set_deleted_key(1);
    }
};

class MyGoogleDenseHashSet : public google::dense_hash_set<size_t> {
public:
    MyGoogleDenseHashSet() : google::dense_hash_set<size_t>() {
        set_empty_key(0);
        set_deleted_key(1);
    }
};

/*----------------------------------------------------------------------------*/

//! Construct different set types for a generic test class
template <template <typename SetType> class TestClass>
struct TestFactory_Set {
    //! Test the unordered_set from STL TR1
    using UnorderedSet = TestClass<std::unordered_multiset<size_t>>;

    //! Test Google's sparse_hash_set
    using GoogleSparseHashSet = TestClass<MyGoogleSparseHashSet>;

    //! Test Google's dense_hash_set
    using GoogleDenseHashSet = TestClass<MyGoogleDenseHashSet>;

    //! Test spp::sparse_hash_set
    using SppSparseHashSet = TestClass<spp::sparse_hash_set<size_t>>;

    //! Test tsl::hopscotch_set
    using TslHopscotchSet = TestClass<tsl::hopscotch_set<size_t>>;

    //! Test tsl::robin_set
    using TslRobinSet = TestClass<tsl::robin_set<size_t>>;

    //! Test robin_hood::unordered_set
    using RobinHoodSet = TestClass<robin_hood::unordered_set<size_t>>;

    //! Test absl::flat_hash_set
    using AbslFlatHashSet = TestClass<absl::flat_hash_set<size_t>>;

    //! Test absl::node_hash_set
    using AbslNodeHashSet = TestClass<absl::node_hash_set<size_t>>;

    //! Run tests on all set types
    void call_testrunner(size_t size);
};

/******************************************************************************/
// Map Benchmarks

//! Test a generic map type with insertions
template <typename MapType>
class Test_Map_Insert : public Benchmark {
public:
    Test_Map_Insert(size_t size, const char* container)
        : Benchmark(size, container) {
    }

    const char* name() const final {
        return "map_insert";
    }

    void run() {
        MapType map;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = adjust(rng());
            map.insert(std::make_pair(r, r));
        }

        die_unless(static_cast<size_t>(map.size()) == size_);
    }
};

//! Test a generic map type with insert, find and delete sequences
template <typename MapType>
class Test_Map_InsertFindDelete : public Benchmark {
public:
    Test_Map_InsertFindDelete(size_t size, const char* container)
        : Benchmark(size, container) {
    }

    const char* name() const final {
        return "map_insert_find_delete";
    }

    void run() {
        MapType map;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = adjust(rng());
            map.insert(std::make_pair(r, r));
        }

        die_unless(static_cast<size_t>(map.size()) == size_);

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            map.find(adjust(rng()));

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = adjust(rng());
            map.erase(map.find(r));
        }

        die_unless(map.empty());
    }
};

//! Test a generic map type with insert, find and delete sequences
template <typename MapType>
class Test_Map_Find : public Benchmark {
public:
    MapType map;

    const char* name() const final {
        return "map_find";
    }

    Test_Map_Find(size_t size, const char* container)
        : Benchmark(size, container) {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = adjust(rng());
            map.insert(std::make_pair(r, r));
        }

        die_unless(static_cast<size_t>(map.size()) == size_);
    }

    void run() {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            map.find(adjust(rng()));
    }
};

/*----------------------------------------------------------------------------*/
// Map Adapters

class MyGoogleSparseHashMap : public google::sparse_hash_map<size_t, size_t> {
public:
    MyGoogleSparseHashMap() : google::sparse_hash_map<size_t, size_t>() {
        set_deleted_key(1);
    }
};

class MyGoogleDenseHashMap : public google::dense_hash_map<size_t, size_t> {
public:
    MyGoogleDenseHashMap() : google::dense_hash_map<size_t, size_t>() {
        set_empty_key(0);
        set_deleted_key(1);
    }
};

class MyRobinHoodMap : public robin_hood::unordered_map<size_t, size_t> {
public:
    using Super = robin_hood::unordered_map<size_t, size_t>;
    auto insert(const std::pair<size_t, size_t>& p) {
        return Super::insert(
            robin_hood::pair<size_t, size_t>(p.first, p.second));
    }
};

/*----------------------------------------------------------------------------*/

//! Construct different map types for a generic test class
template <template <typename MapType> class TestClass>
struct TestFactory_Map {
    //! Test the unordered_map from STL
    using UnorderedMap = TestClass<std::unordered_multimap<size_t, size_t>>;

    //! Test Google's sparse_hash_map
    using GoogleSparseHashMap = TestClass<MyGoogleSparseHashMap>;

    //! Test Google's dense_hash_map
    using GoogleDenseHashMap = TestClass<MyGoogleDenseHashMap>;

    //! Test spp::sparse_hash_map
    using SppSparseHashMap = TestClass<spp::sparse_hash_map<size_t, size_t>>;

    //! Test tsl::robin_map
    using TslRobinMap = TestClass<tsl::robin_map<size_t, size_t>>;

    //! Test tsl::hopscotch_map
    using TslHopscotchMap = TestClass<tsl::hopscotch_map<size_t, size_t>>;

    //! Test robin_hood::unordered_map
    using RobinHoodMap = TestClass<MyRobinHoodMap>;

    //! Test absl::flat_hash_map
    using AbslFlatHashMap = TestClass<absl::flat_hash_map<size_t, size_t>>;

    //! Test absl::node_hash_map
    using AbslNodeHashMap = TestClass<absl::node_hash_map<size_t, size_t>>;

    //! Run tests on all map types
    void call_testrunner(size_t size);
};

/******************************************************************************/

size_t s_repetitions = 0;

//! Repeat (short) tests until enough time elapsed and divide by the repeat.
template <typename TestClass>
void testrunner_loop(size_t size, const char* container_name) {
    std::cerr << "Run benchmark on " << container_name << " size " << size
              << std::endl;

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

    for (size_t r = 0; r < std::max<size_t>(4, target_items / size); ++r)
        mbm.run_print(TestClass(size, container_name));
}

template <template <typename Type> class TestClass>
void TestFactory_Set<TestClass>::call_testrunner(size_t size) {
    tlx::unused(size);

#if MBM_SET_ALGORITHM == 1
    testrunner_loop<UnorderedSet>(size, "std::unordered_multiset");
#elif MBM_SET_ALGORITHM == 2
    testrunner_loop<GoogleSparseHashSet>(size, "google::sparse_hash_set");
#elif MBM_SET_ALGORITHM == 3
    testrunner_loop<GoogleDenseHashSet>(size, "google::dense_hash_set");
#elif MBM_SET_ALGORITHM == 4
    testrunner_loop<SppSparseHashSet>(size, "spp::sparse_hash_set");
#elif MBM_SET_ALGORITHM == 5
    testrunner_loop<TslHopscotchSet>(size, "tsl::hopscotch_set");
#elif MBM_SET_ALGORITHM == 6
    testrunner_loop<TslRobinSet>(size, "tsl::robin_set");
#elif MBM_SET_ALGORITHM == 7
    testrunner_loop<RobinHoodSet>(size, "robin_hood::unordered_set");
#elif MBM_SET_ALGORITHM == 10
    testrunner_loop<AbslFlatHashSet>(size, "absl::flat_hash_set");
#elif MBM_SET_ALGORITHM == 11
    testrunner_loop<AbslNodeHashSet>(size, "absl::node_hash_set");
#endif
}

template <template <typename Type> class TestClass>
void TestFactory_Map<TestClass>::call_testrunner(size_t size) {
    tlx::unused(size);

#if MBM_MAP_ALGORITHM == 1
    testrunner_loop<UnorderedMap>(size, "std::unordered_multimap");
#elif MBM_MAP_ALGORITHM == 2
    testrunner_loop<GoogleSparseHashMap>(size, "google::sparse_hash_map");
#elif MBM_MAP_ALGORITHM == 3
    testrunner_loop<GoogleDenseHashMap>(size, "google::dense_hash_map");
#elif MBM_MAP_ALGORITHM == 4
    testrunner_loop<SppSparseHashMap>(size, "spp::sparse_hash_map");
#elif MBM_MAP_ALGORITHM == 5
    testrunner_loop<TslHopscotchMap>(size, "tsl::hopscotch_map");
#elif MBM_MAP_ALGORITHM == 6
    testrunner_loop<TslRobinMap>(size, "tsl::robin_map");
#elif MBM_MAP_ALGORITHM == 7
    testrunner_loop<RobinHoodMap>(size, "robin_hood::unordered_map");
#elif MBM_MAP_ALGORITHM == 10
    testrunner_loop<AbslFlatHashMap>(size, "absl::flat_hash_map");
#elif MBM_MAP_ALGORITHM == 11
    testrunner_loop<AbslNodeHashMap>(size, "absl::node_hash_map");
#endif
}

/******************************************************************************/

int main() {
    { // Set - speed test only insertion
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "set: insert " << items << "\n";
            TestFactory_Set<Test_Set_Insert>().call_testrunner(items);
        }
    }
    { // Set - speed test insert, find and delete
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "set: insert, find, delete " << items << "\n";
            TestFactory_Set<Test_Set_InsertFindDelete>().call_testrunner(items);
        }
    }
    { // Set - speed test find only
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "set: find " << items << "\n";
            TestFactory_Set<Test_Set_Find>().call_testrunner(items);
        }
    }

    { // Map - speed test only insertion
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "map: insert " << items << "\n";
            TestFactory_Map<Test_Map_Insert>().call_testrunner(items);
        }
    }
    { // Map - speed test insert, find and delete
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "map: insert, find, delete " << items << "\n";
            TestFactory_Map<Test_Map_InsertFindDelete>().call_testrunner(items);
        }
    }
    { // Map - speed test find only
        s_repetitions = 0;

        for (size_t items = min_items; items <= max_items; items *= 2) {
            std::cout << "map: find " << items << "\n";
            TestFactory_Map<Test_Map_Find>().call_testrunner(items);
        }
    }

    return 0;
}

/******************************************************************************/
