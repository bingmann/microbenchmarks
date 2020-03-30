/*******************************************************************************
 * tlx/sort/parallel_radixsort.hpp
 *
 * Parallel radix sort with work-balancing.
 *
 * The set of values is sorted using a 8- or 16-bit radix sort
 * algorithm. Recursive sorts are processed in parallel using a lock-free job
 * queue and OpenMP threads. Two radix sort implementations are used:
 * sequential in-place and parallelized out-of-place.
 *
 * The sequential radix sort is implemented using an explicit recursion stack,
 * which enables threads to "free up" work from the top of the stack when other
 * threads become idle. This variant uses in-place permuting and a character
 * cache (oracle).
 *
 * To parallelize sorting of large buckets an out-of-place variant is
 * implemented using a sequence of three Jobs: count, distribute and
 * copyback. All threads work on a job-specific context called BigRadixStepCE,
 * which encapsules all variables of an 8-bit or 16-bit radix sort (templatized
 * with key_type).
 *
 ******************************************************************************/

#ifndef PRS_PARALLEL_MSD_RADIX_SORT_HEADER
#define PRS_PARALLEL_MSD_RADIX_SORT_HEADER

#include <vector>

#include "shadow_set.hpp"

#include <tlx/logger.hpp>
#include <tlx/thread_pool.hpp>
#include <tlx/multi_timer.hpp>
#include <tlx/die.hpp>

namespace tlx {
namespace parallel_radixsort_detail {

template <typename ValueType, typename KeyType>
inline
KeyType get_key(const ValueType& v, size_t depth)
{
    return v >>
        (8*sizeof(ValueType) - 8*sizeof(KeyType) * depth - 8*sizeof(KeyType));
}


/// Return traits of key_type
template <typename CharT>
class key_traits
{ };

template <>
class key_traits<uint8_t>
{
public:
    static const size_t radix = 256;
    static const size_t add_depth = 1;
};

template <>
class key_traits<uint16_t>
{
public:
    static const size_t radix = 65536;
    static const size_t add_depth = 2;
};

/******************************************************************************/
//! Parallel Radix Sort Parameter Struct

template <typename Iterator_, typename key_type_,
         key_type_ (*key_extractor_)(
                 const typename std::iterator_traits<Iterator_>::value_type& v,
                 size_t depth)>
class PRSParametersDefault
{
public:
    //! key type for radix sort: 8-bit or 16-bit
    typedef key_type_  key_type;

    //! type of the iterator for accessing data to be sorted
    typedef Iterator_ Iterator;

    //! data type for radix sort: int32_t, int64_t, float,...
    typedef typename std::iterator_traits<Iterator>::value_type value_type;

    static const bool debug_steps = false;
    static const bool debug_jobs = false;

    static const bool debug_bucket_size = false;
    static const bool debug_recursion = false;

    static const bool debug_result = false;

    static const bool enable_parallel_radix_sort = true;

    //! enable work freeing
    static const bool enable_work_sharing = true;

    //! whether the base sequential_threshold() on the remaining unsorted data
    //! set or on the whole data set.
    static const bool enable_rest_size = false;

    //! threshold to switch to insertion sort
    static const size_t subsort_threshold = 32;

    //! whether to use in-place sequential sort
    static const bool inplace_sequential_sort = false;

    //! comparator for the sub-sorter
    constexpr static std::less<value_type> cmp = std::less<value_type>();

    //! sub-sorter
    constexpr static auto sub_sort = std::sort<Iterator, std::less<value_type>>;
    // constexpr static void (*sub_sort)(Iterator, Iterator, std::less<value_type>)
    //     = std::sort<Iterator, std::less<value_type>>;

    // constexpr static key_type (*key_extractor)(value_type, size_t) = key_extractor_;
    constexpr static auto key_extractor = key_extractor_;
};


/******************************************************************************/
//! Parallel Radix Sort Context

template <typename Parameters>
class PRSContext : public Parameters
{
public:
    //! total size of input
    size_t totalsize;

    //! number of remaining elements to sort
    std::atomic<size_t> rest_size;

    //! timers for individual sorting steps
    MultiTimer mtimer;

    //! number of threads overall
    size_t num_threads;

    //! thread pool
    ThreadPool threads_;

    //! maximum depth from which to switch to sub_sort
    size_t max_depth;

    //! context constructor
    PRSContext(size_t _thread_num, size_t _max_depth)
        : num_threads(_thread_num),
          threads_(_thread_num),
          max_depth(_max_depth)
    { }

    //! enqueue a new job in the thread pool
    template <typename DataPtr>
    void enqueue(const DataPtr& dptr, size_t depth);

    //! enqueue a new sequential sort job in the thread pool
    template <typename DataPtr>
    void enqueue_small_job(const DataPtr& dptr, size_t depth);

    //! return sequential sorting threshold
    size_t sequential_threshold() {
        // size_t threshold = this->smallsort_threshold;
        size_t threshold = this->subsort_threshold;
        if (this->enable_rest_size) {
            return std::max(threshold, rest_size / num_threads);
        }
        else {
            return std::max(threshold, totalsize / num_threads);
        }
    }

    //! decrement number of unordered elements
    void donesize(size_t n) {
        if (this->enable_rest_size)
            rest_size -= n;
    }
};

// ****************************************************************************
// *** SmallsortJob - sort radix in-place with explicit stack-based recursion

template <typename Context, typename BktSizeType, typename DataPtr>
struct SmallsortJob final
{
    DataPtr dptr;
    size_t  depth;

    typedef BktSizeType bktsize_type;

    typedef typename Context::key_type key_type;
    typedef typename Context::value_type value_type;

    typedef typename DataPtr::DataSet DataSet;
    typedef typename DataSet::Iterator Iterator;

    const static size_t numbkts = key_traits<key_type>::radix;

    SmallsortJob(Context& ctx, const DataPtr& _dptr, size_t _depth)
        : dptr(_dptr), depth(_depth)
    {
        ctx.threads_.enqueue([this, &ctx]() { this->run(ctx); });
    }

    struct RadixStep_CI
    {
        DataPtr      dptr;
        size_t       idx;
        bktsize_type bkt[numbkts + 1];

        // TODO find out why compiler was complaining about `noexcept`
        RadixStep_CI(const DataPtr& _dptr, size_t depth) noexcept
            : dptr(_dptr)
        {
            DataSet ds = dptr.active();
            size_t n = ds.size();

            // count character occurances
            bktsize_type bktsize[numbkts];
            memset(bktsize, 0, sizeof(bktsize));
            for (Iterator i = ds.begin(); i != ds.end(); ++i)
                ++bktsize[static_cast<size_t>(Context::key_extractor(*i, depth))];

            if (Context::inplace_sequential_sort) {
                // inclusive prefix sum
                bkt[0] = bktsize[0];
                bktsize_type last_bkt_size = bktsize[0];
                for (size_t i = 1; i < numbkts; ++i) {
                    bkt[i] = bkt[i - 1] + bktsize[i];
                    if (bktsize[i]) last_bkt_size = bktsize[i];
                }

                // premute in-place
                for (size_t i = 0, j; i < n - last_bkt_size; )
                {
                    value_type perm = std::move(*(ds.begin() + i));
                    key_type permch = Context::key_extractor(perm, depth);
                    while ((j = --bkt[static_cast<size_t>(permch)]) > i)
                    {
                        std::swap(perm, *(ds.begin() + j));
                        permch = Context::key_extractor(perm, depth);
                    }
                    *(ds.begin() + i) = std::move(perm);
                    i += bktsize[static_cast<size_t>(permch)];
                }
            }
            else {
                // exclusive prefix sum
                bkt[0] = 0;
                for (size_t i = 1; i < numbkts; ++i) {
                    bkt[i] = bkt[i - 1] + bktsize[i - 1];
                }

                // distribute
                DataSet sorted = dptr.shadow();
                for (Iterator i = ds.begin(); i != ds.end(); ++i)
                    *(sorted.begin() + bkt[Context::key_extractor(*i, depth)]++) = std::move(*i);

                dptr = dptr.flip(0, n);
                dptr = dptr.copy_back();
            }

            // fix prefix sum
            bkt[0] = 0;
            for (size_t i = 1; i <= numbkts; ++i) {
                bkt[i] = bkt[i - 1] + bktsize[i - 1];
            }
            assert(bkt[numbkts] == n);

            idx = 0;
        }
    };

    void run(Context& ctx)
    {
        size_t n = dptr.size();

        LOGC(ctx.debug_jobs)
            << "Process SmallsortJob " << this << " of size " << n;

        dptr = dptr.copy_back();

        if (n < ctx.subsort_threshold) {
            // FIXME maybe start from depth `depth`
            // insertion_sort(dptr.copy_back(), depth, 0);
            DataSet ds = dptr.copy_back().active();
            ctx.sub_sort(ds.begin(), ds.end(), ctx.cmp);
            ctx.donesize(n);

            delete this;
            return;
        }

        // std::deque is much slower than std::vector, so we use an artificial
        // pop_front variable.
        size_t pop_front = 0;
        std::vector<RadixStep_CI> radixstack;
        radixstack.emplace_back(dptr, depth);

        while (radixstack.size() > pop_front)
        {
            while (radixstack.back().idx < numbkts)
            {
                if (depth + radixstack.size() >= ctx.max_depth) {
                    ctx.donesize(radixstack.back().dptr.size());
                    break;
                }

                RadixStep_CI& rs = radixstack.back();
                size_t b = rs.idx++; // process the bucket rs.idx

                size_t bktsize = rs.bkt[b + 1] - rs.bkt[b];

                if (bktsize == 0)
                    continue;
                else if (bktsize == 1)
                {
                    ctx.donesize(1);
                    continue;
                }
                else if (bktsize < ctx.subsort_threshold)
                {
                    // FIXME maybe start from depth `depth`
                    DataSet ds = rs.dptr.sub(rs.bkt[b], bktsize).copy_back().active();
                    ctx.sub_sort(ds.begin(), ds.end(), ctx.cmp);
                    ctx.donesize(bktsize);
                }
                else
                {
                    radixstack.emplace_back(rs.dptr.sub(rs.bkt[b], bktsize),
                                            depth + radixstack.size());
                }

                if (ctx.enable_work_sharing && ctx.threads_.has_idle())
                {
                    // convert top level of stack into independent jobs
                    LOGC(ctx.debug_jobs)
                        << "Freeing top level of SmallsortJob's radixsort stack";

                    // take out top level step and shorten current stack
                    RadixStep_CI& rt = radixstack[pop_front++];

                    while (rt.idx < numbkts)
                    {
                        b = rt.idx++; // enqueue the bucket rt.idx

                        size_t bktsize_ = rt.bkt[b + 1] - rt.bkt[b];

                        if (bktsize_ == 0)
                            continue;
                        else if (bktsize_ == 1) {
                            ctx.donesize(1);
                            continue;
                        }
                        ctx.enqueue_small_job(rt.dptr.sub(rt.bkt[b], bktsize_),
                                depth + pop_front);
                    }
                }
            }
            radixstack.pop_back();
        }

        delete this;
    }
};

// ****************************************************************************
// *** BigRadixStepCE out-of-place 8- or 16-bit parallel radix sort with Jobs

template <typename Context, typename DataPtr>
struct BigRadixStepCE
{
    typedef typename Context::key_type key_type;
    typedef typename Context::value_type value_type;

    typedef typename DataPtr::DataSet DataSet;
    typedef typename DataSet::Iterator Iterator;

    static const size_t numbkts = key_traits<key_type>::radix;

    DataPtr             dptr;
    size_t              depth;

    size_t              parts;
    size_t              psize;
    std::atomic<size_t> pwork;
    size_t              * bkt;

    BigRadixStepCE(Context& ctx, const DataPtr& dptr, size_t _depth);

    void                count(size_t p, Context& ctx);
    void                count_finished(Context& ctx);

    void                distribute(size_t p, Context& ctx);
    void                distribute_finished(Context& ctx);
};

template <typename Context, typename DataPtr>
BigRadixStepCE<Context, DataPtr>::BigRadixStepCE(
    Context& ctx, const DataPtr& _dptr, size_t _depth)
    : dptr(_dptr), depth(_depth)
{
    size_t n = dptr.size();

    parts = (n + ctx.sequential_threshold() - 1) / ctx.sequential_threshold();
    if (parts == 0) parts = 1;

    psize = (n + parts - 1) / parts;

    LOGC(ctx.debug_jobs)
        << "Area split into " << parts << " parts of size " << psize;

    bkt = new size_t[numbkts * parts + 1];

    // create worker jobs
    pwork = parts;
    for (size_t p = 0; p < parts; ++p)
        ctx.threads_.enqueue([this, p, &ctx]() { count(p, ctx); });
}

template <typename Context, typename DataPtr>
void BigRadixStepCE<Context, DataPtr>::count(size_t p, Context& ctx)
{
    LOGC(ctx.debug_jobs)
        << "Process CountJob " << p << " @ " << this;

    const DataSet& ds = dptr.active();
    Iterator itB = ds.begin() + p * psize;
    Iterator itE = ds.begin() + std::min((p + 1) * psize, dptr.size());
    if (itE < itB) itE = itB;

    size_t mybkt[numbkts] = { 0 };
    for (Iterator it = itB; it != itE; ++it)
        ++mybkt[Context::key_extractor(*it, depth)];

    memcpy(bkt + p * numbkts, mybkt, sizeof(mybkt));

    if (--pwork == 0)
        count_finished(ctx);
}

template <typename Context, typename DataPtr>
void BigRadixStepCE<Context, DataPtr>::count_finished(Context& ctx)
{
    LOGC(ctx.debug_jobs)
        << "Finishing CountJob " << this << " with prefixsum";

    // inclusive prefix sum over bkt
    size_t sum = 0;
    for (size_t i = 0; i < numbkts; ++i)
    {
        for (size_t p = 0; p < parts; ++p)
        {
            bkt[p * numbkts + i] = (sum += bkt[p * numbkts + i]);
        }
    }
    assert(sum == dptr.size());

    // create new jobs
    pwork = parts;
    for (size_t p = 0; p < parts; ++p)
        ctx.threads_.enqueue([this, p, &ctx]() { distribute(p, ctx); });
}

template <typename Context, typename DataPtr>
void BigRadixStepCE<Context, DataPtr>::distribute(size_t p, Context& ctx)
{
    LOGC(ctx.debug_jobs)
        << "Process DistributeJob " << p << " @ " << this;

    const DataSet& ds = dptr.active();
    Iterator itB = ds.begin() + p * psize;
    Iterator itE = ds.begin() + std::min((p + 1) * psize, dptr.size());
    if (itE < itB) itE = itB;

    Iterator sorted = dptr.shadow().begin(); // get alternative shadow pointer array

    size_t mybkt[numbkts];
    memcpy(mybkt, bkt + p * numbkts, sizeof(mybkt));

    for (Iterator it = itB; it != itE; ++it)
        sorted[--mybkt[Context::key_extractor(*it, depth)]] = std::move(*it);

    if (p == 0) // these are needed for recursion into bkts
        memcpy(bkt, mybkt, sizeof(mybkt));

    if (--pwork == 0)
        distribute_finished(ctx);
}

template <typename Context, typename DataPtr>
void BigRadixStepCE<Context, DataPtr>::distribute_finished(Context& ctx)
{
    LOGC(ctx.debug_jobs)
        << "Finishing DistributeJob " << this << " with enqueuing subjobs";

    // TODO check for correctness
    // first p's bkt pointers are boundaries between bkts, just add sentinel:
    assert(bkt[0] == 0);
    bkt[numbkts] = dptr.size();

    for (size_t i = 0; i < numbkts; ++i)
    {
        if (bkt[i] == bkt[i + 1])
            continue;
        else if (bkt[i] + 1 == bkt[i + 1]) { // just one element, copyback
            dptr.flip(bkt[i], 1).copy_back();
            ctx.donesize(1);
        }
        else
            ctx.enqueue(dptr.flip(bkt[i], bkt[i + 1] - bkt[i]), depth + 1);
    }

    delete[] bkt;
    delete this;
}

// ****************************************************************************
// *** PRSContext::enqueue() and PRSContext::enqueue_small_job()

template <typename Parameters>
template <typename DataPtr>
void PRSContext<Parameters>::enqueue_small_job(const DataPtr& dptr, size_t depth)
{
    using Context = PRSContext<Parameters>;

    if (dptr.size() < (1LLU << 32))
        new SmallsortJob<Context, uint32_t, DataPtr>(*this, dptr, depth);
    else
        new SmallsortJob<Context, uint64_t, DataPtr>(*this, dptr, depth);
}


template <typename Parameters>
template <typename DataPtr>
void PRSContext<Parameters>::enqueue(const DataPtr& dptr, size_t depth)
{
    using Context = PRSContext<Parameters>;

    if (this->enable_parallel_radix_sort
     && dptr.size() > sequential_threshold()) {
        new BigRadixStepCE<Context, DataPtr>(*this, dptr, depth);
    }
    else {
        enqueue_small_job(dptr, depth);
    }
}

/******************************************************************************/
// Frontends

template <typename PRSParameters, typename Iterator>
static inline
void radix_sort_params(Iterator begin, Iterator end, size_t max_depth)
{
    using Context = PRSContext<PRSParameters>;
    using Type = typename std::iterator_traits<Iterator>::value_type;

    Context ctx(std::thread::hardware_concurrency(), max_depth);
    ctx.totalsize = end - begin;
    ctx.rest_size = ctx.totalsize;

    // allocate shadow pointer array
    Type *shadow = new Type[ctx.totalsize];
    Iterator shadow_begin = Iterator(shadow);

    ctx.enqueue(ShadowDataPtr<DummyDataSet<Iterator>>(begin, end,
                shadow_begin, shadow_begin + ctx.totalsize), 0);

    ctx.threads_.loop_until_empty();

    assert(!ctx.enable_rest_size || ctx.rest_size == 0);

    delete[] shadow;
}

/*!
 * Radix sort the iterator range [begin,end). Sort unconditionally up to depth
 * max_depth, then call the sub_sort method for further sorting. Small buckets
 * are sorted using std::sort() with given comparator. Characters are extracted
 * from items in the range using the at_radix(depth) method.
 */
template <typename Iterator,
    uint8_t (*key_extractor)(
        const typename std::iterator_traits<Iterator>::value_type&, size_t)>
static inline
void radix_sort(Iterator begin, Iterator end, size_t max_depth)
{
    radix_sort_params<PRSParametersDefault<Iterator, uint8_t, key_extractor>, Iterator>(
            begin, end, max_depth);
}


template <typename Iterator,
    uint16_t (*key_extractor)(
        const typename std::iterator_traits<Iterator>::value_type&, size_t)>
static inline
void radix_sort(Iterator begin, Iterator end, size_t max_depth)
{
    radix_sort_params<PRSParametersDefault<Iterator, uint16_t, key_extractor>, Iterator>(
            begin, end, max_depth);
}

} // namespace parallel_radixsort_detail
} // namespace tlx

#endif // !PRS_PARALLEL_MSD_RADIX_SORT_HEADER

/******************************************************************************/
