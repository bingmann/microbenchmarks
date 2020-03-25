/*******************************************************************************
 * mbm_trees.cpp
 *
 * Microbenchmark insertion, find, and delete in ordered trees
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

#include <set>
#include <tlx/container/btree_multiset.hpp>
#include <tlx/container/splay_tree.hpp>
#include <unordered_set>

#include <map>
#include <tlx/container/btree_multimap.hpp>
#include <unordered_map>

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

//! Traits used for the speed tests, BTREE_DEBUG is not defined.
template <int InnerSlots, int LeafSlots>
struct btree_traits_speed : tlx::btree_default_traits<size_t, size_t> {
    static const bool self_verify = false;
    static const bool debug = false;

    static const int leaf_slots = InnerSlots;
    static const int inner_slots = LeafSlots;
};

/******************************************************************************/

class TreeBenchmark {
public:
    TreeBenchmark(size_t size, const char* container)
        : size_(size), container_(container) {
    }

    size_t size_;
    const char* container_;

    virtual const char* name() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const TreeBenchmark& b) {
        return os << "benchmark=" << b.name() << '\t'
                  << "container=" << b.container_ << '\t' << "size=" << b.size_
                  << '\t';
    }
};

/******************************************************************************/
// Set Benchmarks

//! Test a generic set type with insertions
template <typename SetType>
class Test_Set_Insert : public TreeBenchmark {
public:
    Test_Set_Insert(size_t size, const char* container)
        : TreeBenchmark(size, container) {
    }

    const char* name() const final {
        return "set_insert";
    }

    void run() {
        SetType set;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(rng());

        die_unless(set.size() == size_);
    }
};

//! Test a generic set type with insert, find and delete sequences
template <typename SetType>
class Test_Set_InsertFindDelete : public TreeBenchmark {
public:
    Test_Set_InsertFindDelete(size_t size, const char* container)
        : TreeBenchmark(size, container) {
    }

    const char* name() const final {
        return "set_insert_find_delete";
    }

    void run() {
        SetType set;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(rng());

        die_unless(set.size() == size_);

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            set.find(rng());

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            set.erase(set.find(rng()));

        die_unless(set.empty());
    }
};

//! Test a generic set type with insert, find and delete sequences
template <typename SetType>
class Test_Set_Find : public TreeBenchmark {
public:
    SetType set;

    const char* name() const final {
        return "set_find";
    }

    Test_Set_Find(size_t size, const char* container)
        : TreeBenchmark(size, container) {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.insert(rng());

        die_unless(set.size() == size_);
    }

    void run() {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            set.find(rng());
    }
};

//! Construct different set types for a generic test class
template <template <typename SetType> class TestClass>
struct TestFactory_Set {
    //! Test the multiset red-black tree from STL
    typedef TestClass<std::multiset<size_t>> StdSet;

    //! Test the multiset red-black tree from STL
    typedef TestClass<tlx::splay_multiset<size_t>> SplaySet;

    //! Test the unordered_set from STL TR1
    typedef TestClass<std::unordered_multiset<size_t>> UnorderedSet;

    //! Test the B+ tree with a specific leaf/inner slots
    template <int Slots>
    struct BtreeSet : TestClass<tlx::btree_multiset<size_t, std::less<size_t>,
                          struct btree_traits_speed<Slots, Slots>>> {
        BtreeSet(size_t n, const char* cn)
            : TestClass<tlx::btree_multiset<size_t, std::less<size_t>,
                  struct btree_traits_speed<Slots, Slots>>>(n, cn) {}
    };

    //! Run tests on all set types
    void call_testrunner(size_t size);
};

/******************************************************************************/
// Map Benchmarks

//! Test a generic map type with insertions
template <typename MapType>
class Test_Map_Insert : public TreeBenchmark {
public:
    Test_Map_Insert(size_t size, const char* container)
        : TreeBenchmark(size, container) {
    }

    const char* name() const final {
        return "map_insert";
    }

    void run() {
        MapType map;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = rng();
            map.insert(std::make_pair(r, r));
        }

        die_unless(map.size() == size_);
    }
};

//! Test a generic map type with insert, find and delete sequences
template <typename MapType>
class Test_Map_InsertFindDelete : public TreeBenchmark {
public:
    Test_Map_InsertFindDelete(size_t size, const char* container)
        : TreeBenchmark(size, container) {
    }

    const char* name() const final {
        return "map_insert_find_delete";
    }

    void run() {
        MapType map;

        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = rng();
            map.insert(std::make_pair(r, r));
        }

        die_unless(map.size() == size_);

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            map.find(rng());

        rng.seed(seed);
        for (size_t i = 0; i < size_; i++)
            map.erase(map.find(rng()));

        die_unless(map.empty());
    }
};

//! Test a generic map type with insert, find and delete sequences
template <typename MapType>
class Test_Map_Find : public TreeBenchmark {
public:
    MapType map;

    const char* name() const final {
        return "map_find";
    }

    Test_Map_Find(size_t size, const char* container)
        : TreeBenchmark(size, container) {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++) {
            size_t r = rng();
            map.insert(std::make_pair(r, r));
        }

        die_unless(map.size() == size_);
    }

    void run() {
        std::default_random_engine rng(seed);
        for (size_t i = 0; i < size_; i++)
            map.find(rng());
    }
};

//! Construct different map types for a generic test class
template <template <typename MapType> class TestClass>
struct TestFactory_Map {
    //! Test the multimap red-black tree from STL
    typedef TestClass<std::multimap<size_t, size_t>> StdMap;

    //! Test the unordered_map from STL
    typedef TestClass<std::unordered_multimap<size_t, size_t>> UnorderedMap;

    //! Test the B+ tree with a specific leaf/inner slots
    template <int Slots>
    struct BtreeMap
        : TestClass<tlx::btree_multimap<size_t, size_t, std::less<size_t>,
              struct btree_traits_speed<Slots, Slots>>> {
        BtreeMap(size_t n, const char* cn)
            : TestClass<tlx::btree_multimap<size_t, size_t, std::less<size_t>,
                  struct btree_traits_speed<Slots, Slots>>>(n, cn) {}
    };

    //! Run tests on all map types
    void call_testrunner(size_t size);
};

/******************************************************************************/

size_t s_repetitions = 0;

//! Repeat (short) tests until enough time elapsed and divide by the repeat.
template <typename TestClass>
void testrunner_loop(size_t size, const char* container_name) {

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

    testrunner_loop<StdSet>(size, "std::multiset");
    testrunner_loop<UnorderedSet>(size, "std::unordered_multiset");
    testrunner_loop<SplaySet>(size, "tlx::splay_multiset");

    testrunner_loop<BtreeSet<4>>(size, "tlx::btree_multiset<4>");
    testrunner_loop<BtreeSet<8>>(size, "tlx::btree_multiset<8>");
    testrunner_loop<BtreeSet<16>>(size, "tlx::btree_multiset<16>");
    testrunner_loop<BtreeSet<32>>(size, "tlx::btree_multiset<32>");
    testrunner_loop<BtreeSet<64>>(size, "tlx::btree_multiset<64>");
    testrunner_loop<BtreeSet<128>>(size, "tlx::btree_multiset<128>");
    testrunner_loop<BtreeSet<256>>(size, "tlx::btree_multiset<256>");
}

template <template <typename Type> class TestClass>
void TestFactory_Map<TestClass>::call_testrunner(size_t size) {

    testrunner_loop<StdMap>(size, "std::multimap");
    testrunner_loop<UnorderedMap>(size, "std::unordered_multimap");

    testrunner_loop<BtreeMap<4>>(size, "tlx::btree_multimap<4>");
    testrunner_loop<BtreeMap<8>>(size, "tlx::btree_multimap<8>");
    testrunner_loop<BtreeMap<16>>(size, "tlx::btree_multimap<16>");
    testrunner_loop<BtreeMap<32>>(size, "tlx::btree_multimap<32>");
    testrunner_loop<BtreeMap<64>>(size, "tlx::btree_multimap<64>");
    testrunner_loop<BtreeMap<128>>(size, "tlx::btree_multimap<128>");
    testrunner_loop<BtreeMap<256>>(size, "tlx::btree_multimap<256>");
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
