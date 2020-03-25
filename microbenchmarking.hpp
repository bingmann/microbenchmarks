/*******************************************************************************
 * microbenchmarking.hpp
 *
 * Framework for easier microbenchmarking with perf events.
 *
 * Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the MIT License in the LICENSE file.
 ******************************************************************************/

#ifndef MICROBENCHMARKING_HEADER
#define MICROBENCHMARKING_HEADER

#include <tlx/logger.hpp>
#include <tlx/timestamp.hpp>

#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>

#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

enum class PerfCache { L1D, L1I, LL, DTLB, ITLB, BPU, Node };
enum class PerfCacheOp { Read, Write, Prefetch };
enum class PerfCacheOpResult { Access, Miss };

class PerfMeasurement {
public:
    PerfMeasurement();
    ~PerfMeasurement();

    /*------------------------------------------------------------------------*/

    //! start all measurements
    void start();

    //! stop measurements
    void stop();

    /*------------------------------------------------------------------------*/

    //! measure PERF_TYPE_HARDWARE / HW_CPU_CYCLES
    bool enable_hw_cpu_cycles();
    uint64_t hw_cpu_cycles();

    //! measure PERF_TYPE_HARDWARE / HW_INSTRUCTIONS
    bool enable_hw_instructions();
    uint64_t hw_instructions();

    //! measure PERF_TYPE_HARDWARE / HW_CACHE_REFERENCES
    bool enable_hw_cache_references();
    uint64_t hw_cache_references();

    //! measure PERF_TYPE_HARDWARE / HW_CACHE_MISSES
    bool enable_hw_cache_misses();
    uint64_t hw_cache_misses();

    //! measure PERF_TYPE_HARDWARE / HW_BRANCH_INSTRUCTIONS
    bool enable_hw_branch_instructions();
    uint64_t hw_branch_instructions();

    //! measure PERF_TYPE_HARDWARE / HW_BRANCH_MISSES
    bool enable_hw_branch_misses();
    uint64_t hw_branch_misses();

    //! measure PERF_TYPE_HARDWARE / HW_BUS_CYCLES
    bool enable_hw_bus_cycles();
    uint64_t hw_bus_cycles();

    //! measure PERF_TYPE_HARDWARE / HW_REF_CPU_CYCLES
    bool enable_hw_ref_cpu_cycles();
    uint64_t hw_ref_cpu_cycles();

    /*------------------------------------------------------------------------*/

    //! measure PERF_TYPE_HW_CACHE
    bool enable_hw_cache1(PerfCache cache, PerfCacheOp cache_op,
        PerfCacheOpResult cache_op_result);
    uint64_t hw_cache1();

    //! measure PERF_TYPE_HW_CACHE
    bool enable_hw_cache2(PerfCache cache, PerfCacheOp cache_op,
        PerfCacheOpResult cache_op_result);
    uint64_t hw_cache2();

    //! measure PERF_TYPE_HW_CACHE
    bool enable_hw_cache3(PerfCache cache, PerfCacheOp cache_op,
        PerfCacheOpResult cache_op_result);
    uint64_t hw_cache3();

    /*------------------------------------------------------------------------*/

    //! measure a custom perf type / config
    bool enable_custom1(
        uint32_t type, uint32_t config, const char* name = nullptr);
    uint64_t custom1();

    //! measure a custom perf type / config
    bool enable_custom2(
        uint32_t type, uint32_t config, const char* name = nullptr);
    uint64_t custom2();

protected:
    //! first file descriptor (group leader)
    int fd_ = -1;

    //! file descriptor for HW_CPU_CYCLES
    int fd_hw_cpu_cycles_ = -1;

    //! file descriptor for HW_INSTRUCTIONS
    int fd_hw_instructions_ = -1;

    //! file descriptor for HW_CACHE_REFERENCES
    int fd_hw_cache_references_ = -1;

    //! file descriptor for HW_CACHE_MISSES
    int fd_hw_cache_misses_ = -1;

    //! file descriptor for HW_BRANCH_INSTRUCTIONS
    int fd_hw_branch_instructions_ = -1;

    //! file descriptor for HW_BRANCH_MISSES
    int fd_hw_branch_misses_ = -1;

    //! file descriptor for HW_BUS_CYCLES
    int fd_hw_bus_cycles_ = -1;

    //! file descriptor for HW_REF_CPU_CYCLES
    int fd_hw_ref_cpu_cycles_ = -1;

    //! file descriptor for first cache measurement
    int fd_hw_cache1_ = -1;

    //! first cache measurement level
    PerfCache hw_cache1_;
    //! first cache measurement operation
    PerfCacheOp hw_cache1_op_;
    //! first cache measurement operation result
    PerfCacheOpResult hw_cache1_op_result_;

    //! file descriptor for second cache measurement
    int fd_hw_cache2_ = -1;

    //! second cache measurement level
    PerfCache hw_cache2_;
    //! second cache measurement operation
    PerfCacheOp hw_cache2_op_;
    //! second cache measurement operation result
    PerfCacheOpResult hw_cache2_op_result_;

    //! file descriptor for third cache measurement
    int fd_hw_cache3_ = -1;

    //! third cache measurement level
    PerfCache hw_cache3_;
    //! third cache measurement operation
    PerfCacheOp hw_cache3_op_;
    //! third cache measurement operation result
    PerfCacheOpResult hw_cache3_op_result_;

    //! file descriptor for first custom measurement
    int fd_custom1_ = -1;
    //! RESULT name of first custom measurement
    const char* custom1_name_ = nullptr;

    //! file descriptor for second custom measurement
    int fd_custom2_ = -1;
    //! RESULT name of second custom measurement
    const char* custom2_name_ = nullptr;

    //! initialize perf event collection and return fd
    int enable_event(uint32_t type, uint32_t config);

    //! initialize perf event collection and return fd
    bool enable_event_ref(
        uint32_t type, uint32_t config, int& fd, const char* name);

    //! read fd and return perf measurement
    uint64_t read_fd(int fd) const;

    //! combine PerfCache/PerfCacheOp/PerfCacheOpResult enums
    uint32_t combine_cache_flags(PerfCache cache, PerfCacheOp cache_op,
        PerfCacheOpResult cache_op_result);

    //! print RESULT description of cache flags
    static void print_cache_flags(std::ostream& os, PerfCache cache,
        PerfCacheOp cache_op, PerfCacheOpResult cache_op_result);

    //! maybe close an fd
    void close_fd(int& fd) const;
};

/******************************************************************************/

PerfMeasurement::PerfMeasurement() {
}

PerfMeasurement::~PerfMeasurement() {
    close_fd(fd_);

    close_fd(fd_hw_cpu_cycles_);
    close_fd(fd_hw_instructions_);
    close_fd(fd_hw_cache_references_);
    close_fd(fd_hw_cache_misses_);
    close_fd(fd_hw_branch_instructions_);
    close_fd(fd_hw_branch_misses_);
    close_fd(fd_hw_bus_cycles_);
    close_fd(fd_hw_ref_cpu_cycles_);

    close_fd(fd_hw_cache1_);
    close_fd(fd_hw_cache2_);
    close_fd(fd_hw_cache3_);
    close_fd(fd_custom1_);
    close_fd(fd_custom2_);
}

void PerfMeasurement::start() {
    ioctl(fd_, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(fd_, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

void PerfMeasurement::stop() {
    ioctl(fd_, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
}

/******************************************************************************/

bool PerfMeasurement::enable_hw_cpu_cycles() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES,
        fd_hw_cpu_cycles_, "enable_hw_cpu_cycles");
}

uint64_t PerfMeasurement::hw_cpu_cycles() {
    return read_fd(fd_hw_cpu_cycles_);
}

bool PerfMeasurement::enable_hw_instructions() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS,
        fd_hw_instructions_, "enable_hw_instructions");
}

uint64_t PerfMeasurement::hw_instructions() {
    return read_fd(fd_hw_instructions_);
}

bool PerfMeasurement::enable_hw_cache_references() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES,
        fd_hw_cache_references_, "enable_hw_cache_references");
}

uint64_t PerfMeasurement::hw_cache_references() {
    return read_fd(fd_hw_cache_references_);
}

bool PerfMeasurement::enable_hw_cache_misses() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES,
        fd_hw_cache_misses_, "enable_hw_cache_misses");
}

uint64_t PerfMeasurement::hw_cache_misses() {
    return read_fd(fd_hw_cache_misses_);
}

bool PerfMeasurement::enable_hw_branch_instructions() {
    return enable_event_ref(PERF_TYPE_HARDWARE,
        PERF_COUNT_HW_BRANCH_INSTRUCTIONS, fd_hw_branch_instructions_,
        "enable_hw_branch_instructions");
}

uint64_t PerfMeasurement::hw_branch_instructions() {
    return read_fd(fd_hw_branch_instructions_);
}

bool PerfMeasurement::enable_hw_branch_misses() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES,
        fd_hw_branch_misses_, "enable_hw_branch_misses");
}

uint64_t PerfMeasurement::hw_branch_misses() {
    return read_fd(fd_hw_branch_misses_);
}

bool PerfMeasurement::enable_hw_bus_cycles() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES,
        fd_hw_bus_cycles_, "enable_hw_bus_cycles");
}

uint64_t PerfMeasurement::hw_bus_cycles() {
    return read_fd(fd_hw_bus_cycles_);
}

bool PerfMeasurement::enable_hw_ref_cpu_cycles() {
    return enable_event_ref(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES,
        fd_hw_ref_cpu_cycles_, "enable_hw_ref_cpu_cycles");
}

uint64_t PerfMeasurement::hw_ref_cpu_cycles() {
    return read_fd(fd_hw_ref_cpu_cycles_);
}

/******************************************************************************/

bool PerfMeasurement::enable_hw_cache1(
    PerfCache cache, PerfCacheOp cache_op, PerfCacheOpResult cache_op_result) {
    hw_cache1_ = cache;
    hw_cache1_op_ = cache_op;
    hw_cache1_op_result_ = cache_op_result;
    return enable_event_ref(PERF_TYPE_HW_CACHE,
        combine_cache_flags(cache, cache_op, cache_op_result), fd_hw_cache1_,
        "enable_hw_cache1");
}

uint64_t PerfMeasurement::hw_cache1() {
    return read_fd(fd_hw_cache1_);
}

bool PerfMeasurement::enable_hw_cache2(
    PerfCache cache, PerfCacheOp cache_op, PerfCacheOpResult cache_op_result) {
    hw_cache2_ = cache;
    hw_cache2_op_ = cache_op;
    hw_cache2_op_result_ = cache_op_result;
    return enable_event_ref(PERF_TYPE_HW_CACHE,
        combine_cache_flags(cache, cache_op, cache_op_result), fd_hw_cache2_,
        "enable_hw_cache2");
}

uint64_t PerfMeasurement::hw_cache2() {
    return read_fd(fd_hw_cache2_);
}

bool PerfMeasurement::enable_hw_cache3(
    PerfCache cache, PerfCacheOp cache_op, PerfCacheOpResult cache_op_result) {
    hw_cache3_ = cache;
    hw_cache3_op_ = cache_op;
    hw_cache3_op_result_ = cache_op_result;
    return enable_event_ref(PERF_TYPE_HW_CACHE,
        combine_cache_flags(cache, cache_op, cache_op_result), fd_hw_cache3_,
        "enable_hw_cache3");
}

uint64_t PerfMeasurement::hw_cache3() {
    return read_fd(fd_hw_cache3_);
}

/******************************************************************************/

bool PerfMeasurement::enable_custom1(
    uint32_t type, uint32_t config, const char* name) {
    custom1_name_ = name;
    return enable_event_ref(type, config, fd_custom1_, "enable_custom1");
}

uint64_t PerfMeasurement::custom1() {
    return read_fd(fd_custom1_);
}

bool PerfMeasurement::enable_custom2(
    uint32_t type, uint32_t config, const char* name) {
    custom2_name_ = name;
    return enable_event_ref(type, config, fd_custom2_, "enable_custom2");
}

uint64_t PerfMeasurement::custom2() {
    return read_fd(fd_custom2_);
}

/******************************************************************************/

int PerfMeasurement::enable_event(uint32_t type, uint32_t config) {
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.size = sizeof(attr);
    attr.type = type;
    attr.config = config;

    attr.disabled = 0;
    attr.exclude_user = 0;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.read_format = PERF_FORMAT_ID;

    int fd = syscall(__NR_perf_event_open, &attr,
        /* pid */ 0, /* cpu */ -1, /* group_fd */ fd_,
        /* flags */ PERF_FLAG_FD_CLOEXEC);

    // copy fd as group leader if valid
    if (fd >= 0 && fd_ < 0)
        fd_ = fd;

    return fd;
}

bool PerfMeasurement::enable_event_ref(
    uint32_t type, uint32_t config, int& fd, const char* name) {
    if (fd >= 0)
        return true;

    fd = enable_event(type, config);

    if (fd < 0) {
        LOG1 << "PerfMeasurement::" << name << "() error: " << strerror(errno);
        return false;
    }

    return true;
}

uint64_t PerfMeasurement::read_fd(int fd) const {
    if (fd < 0)
        return uint64_t(-1);

    struct read_format {
        uint64_t value;
        uint64_t id;
    };

    char buffer[32];
    struct read_format* rf = reinterpret_cast<struct read_format*>(buffer);

    ssize_t r = read(fd, buffer, sizeof(buffer));
    if (r != sizeof(read_format))
        return uint64_t(-1);

    uint64_t value = rf->value;
    sLOG0 << value << r;

    return value;
}

uint32_t PerfMeasurement::combine_cache_flags(
    PerfCache cache, PerfCacheOp cache_op, PerfCacheOpResult cache_op_result) {
    uint32_t x = 0;

    switch (cache) {
    case PerfCache::L1D:
        x |= PERF_COUNT_HW_CACHE_L1D;
        break;
    case PerfCache::L1I:
        x |= PERF_COUNT_HW_CACHE_L1I;
        break;
    case PerfCache::LL:
        x |= PERF_COUNT_HW_CACHE_LL;
        break;
    case PerfCache::DTLB:
        x |= PERF_COUNT_HW_CACHE_DTLB;
        break;
    case PerfCache::ITLB:
        x |= PERF_COUNT_HW_CACHE_ITLB;
        break;
    case PerfCache::BPU:
        x |= PERF_COUNT_HW_CACHE_BPU;
        break;
    case PerfCache::Node:
        x |= PERF_COUNT_HW_CACHE_NODE;
        break;
    }

    switch (cache_op) {
    case PerfCacheOp::Read:
        x |= (PERF_COUNT_HW_CACHE_OP_READ << 8);
        break;
    case PerfCacheOp::Write:
        x |= (PERF_COUNT_HW_CACHE_OP_WRITE << 8);
        break;
    case PerfCacheOp::Prefetch:
        x |= (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8);
        break;
    }

    switch (cache_op_result) {
    case PerfCacheOpResult::Access:
        x |= (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
        break;
    case PerfCacheOpResult::Miss:
        x |= (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
        break;
    }

    return x;
}

void PerfMeasurement::print_cache_flags(std::ostream& os, PerfCache cache,
    PerfCacheOp cache_op, PerfCacheOpResult cache_op_result) {

    switch (cache) {
    case PerfCache::L1D:
        os << "l1d_";
        break;
    case PerfCache::L1I:
        os << "l1i_";
        break;
    case PerfCache::LL:
        os << "ll_";
        break;
    case PerfCache::DTLB:
        os << "dtlb_";
        break;
    case PerfCache::ITLB:
        os << "itlb_";
        break;
    case PerfCache::BPU:
        os << "bpu_";
        break;
    case PerfCache::Node:
        os << "node_";
        break;
    }

    switch (cache_op) {
    case PerfCacheOp::Read:
        os << "read_";
        break;
    case PerfCacheOp::Write:
        os << "write_";
        break;
    case PerfCacheOp::Prefetch:
        os << "prefetch_";
        break;
    }

    switch (cache_op_result) {
    case PerfCacheOpResult::Access:
        os << "access";
        break;
    case PerfCacheOpResult::Miss:
        os << "miss";
        break;
    }
}

void PerfMeasurement::close_fd(int& fd) const {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

/******************************************************************************/

class Microbenchmark : public PerfMeasurement {
public:
    template <typename Benchmark>
    void run(Benchmark&& benchmark) {

        double ts1 = tlx::timestamp();
        PerfMeasurement::start();

        benchmark.run();

        PerfMeasurement::stop();
        double ts2 = tlx::timestamp();

        time_ = ts2 - ts1;
    }

    template <typename Benchmark>
    void run_print(Benchmark&& benchmark) {
        run(benchmark);
        print(std::forward<Benchmark>(benchmark));
    }

    template <typename Benchmark>
    void run_check_print(Benchmark&& benchmark) {
        run(benchmark);
        benchmark.check();
        print(std::forward<Benchmark>(benchmark));
    }

    //! run repeated experiment at least this time
    double repeated_min_time_ = 1.0;
    //! shorted repeated experiment if over this time
    double repeated_max_time_ = 2.0;

    template <typename Benchmark, typename... Args>
    void run_auto_repeat(size_t* repetitions, Args&&... args) {
        repetitions_ = *repetitions;
        if (repetitions_ == 0)
            repetitions_ = 1;

        double ts1, ts2;

        while (true) {
            // initialize test structures
            Benchmark benchmark(std::forward<Args>(args)...);

            ts1 = tlx::timestamp();
            PerfMeasurement::start();

            for (size_t r = 0; r < repetitions_; ++r)
                benchmark.run();

            PerfMeasurement::stop();
            ts2 = tlx::timestamp();

            time_ = ts2 - ts1;

            std::cout << "Run with " << repetitions_ << " repetitions "
                      << " in time " << time_ << "\n";

            // discard and repeat if test took less than one second.
            if (time_ < repeated_min_time_) {
                repetitions_ *= 2;
                continue;
            }

            print(benchmark);

            // if too many repetition, divide by two
            if (time_ > repeated_max_time_)
                repetitions_ /= 2;

            *repetitions = repetitions_;
            break;
        }
    }

    template <typename Benchmark>
    void print(Benchmark&& benchmark, std::ostream& os = std::cout) {
        os << "RESULT\t" << benchmark << "time=" << time() << '\t'
           << "repetitions=" << repetitions_ << '\t';

        if (fd_hw_cpu_cycles_ >= 0)
            os << "cpu_cycles=" << hw_cpu_cycles() << '\t';
        if (fd_hw_instructions_ >= 0)
            os << "instructions=" << hw_instructions() << '\t';

        if (fd_hw_cache_references_ >= 0)
            os << "cache_references=" << hw_cache_references() << '\t';
        if (fd_hw_cache_misses_ >= 0)
            os << "cache_misses=" << hw_cache_misses() << '\t';

        if (fd_hw_branch_instructions_ >= 0)
            os << "branch_instructions=" << hw_branch_instructions() << '\t';
        if (fd_hw_branch_misses_ >= 0)
            os << "branch_misses=" << hw_branch_misses() << '\t';

        if (fd_hw_bus_cycles_ >= 0)
            os << "bus_cycles=" << hw_bus_cycles() << '\t';
        if (fd_hw_ref_cpu_cycles_ >= 0)
            os << "ref_cpu_cycles=" << hw_ref_cpu_cycles() << '\t';

        if (fd_hw_cache1_ >= 0) {
            print_cache_flags(
                os, hw_cache1_, hw_cache1_op_, hw_cache1_op_result_);
            os << '=' << hw_cache1() << '\t';
        }
        if (fd_hw_cache2_ >= 0) {
            print_cache_flags(
                os, hw_cache2_, hw_cache2_op_, hw_cache2_op_result_);
            os << '=' << hw_cache2() << '\t';
        }
        if (fd_hw_cache3_ >= 0) {
            print_cache_flags(
                os, hw_cache3_, hw_cache3_op_, hw_cache3_op_result_);
            os << '=' << hw_cache3() << '\t';
        }

        if (fd_custom1_ >= 0) {
            os << (custom1_name_ ? custom1_name_ : "custom1") << '='
               << custom1() << '\t';
        }
        if (fd_custom2_ >= 0) {
            os << (custom2_name_ ? custom2_name_ : "custom2") << '='
               << custom2() << '\t';
        }

        os << '\n';
    }

    double time_ = 0;

    double time() const {
        return time_;
    }

    size_t repetitions_ = 1;
};

#endif // !MICROBENCHMARKING_HEADER

/******************************************************************************/
