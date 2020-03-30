#ifndef SHADOW_SET_H
#define SHADOW_SET_H

#include <cstdint>
#include <cassert>
#include <utility>
#include <memory>

#include <tlx/meta/enable_if.hpp>


namespace tlx {
namespace parallel_radixsort_detail {

template <typename Iterator_>
class DummyDataSet {
public:
    typedef Iterator_ Iterator;

    DummyDataSet(Iterator begin, Iterator end)
        : begin_(begin), end_(end)
    { }

protected:
    Iterator begin_;
    Iterator end_;

public:
    const Iterator begin() const { return begin_; }
    const Iterator end() const { return end_; }

    size_t size() const { return end_ - begin_; }

    DummyDataSet subi(size_t offset, size_t sub_size) const {
        return DummyDataSet(begin_ + offset, begin_ + sub_size);
    }
};

template <typename DataSet_>
class ShadowDataPtr
{
public:
    typedef DataSet_ DataSet;
    typedef typename DataSet::Iterator Iterator;

protected:
    DataSet active_;
    DataSet shadow_;

    bool flipped_;
public:
    ShadowDataPtr(Iterator active_begin, Iterator active_end, 
              Iterator shadow_begin, Iterator shadow_end,
              bool flipped = false)
        : active_(active_begin, active_end),
          shadow_(shadow_begin, shadow_end),
          flipped_(flipped)
    { }

    ShadowDataPtr(DataSet active, DataSet shadow, bool flipped = false)
        : active_(active), shadow_(shadow), flipped_(flipped)
    { }

    // virtual ~ShadowDataPtr();

    //! return currently active array
    const DataSet& active() const { return active_; }

    //! return current shadow array
    const DataSet& shadow() const { return shadow_; }

    //! true if flipped to back array
    bool flipped() const { return flipped_; }

    //! return valid length
    size_t size() const { return active_.size(); }

    //! Advance (both) pointers by given offset, return sub-array without flip
    ShadowDataPtr sub(size_t offset, size_t sub_size) const {
        assert(offset + sub_size <= size());
        return ShadowDataPtr(active_.subi(offset, offset + sub_size),
                        shadow_.subi(offset, offset + sub_size),
                        flipped_);
    }

    //! construct a ShadowDataPtr object specifying a sub-array with flipping
    //! to other array.
    ShadowDataPtr flip(size_t offset, size_t sub_size) const {
        assert(offset + sub_size <= size());
        return ShadowDataPtr(shadow_.subi(offset, offset + sub_size),
                         active_.subi(offset, offset + sub_size),
                         !flipped_);
    }

    //! return subarray pointer to n elements in original array, might copy from
    //! shadow before returning.
    ShadowDataPtr copy_back() const {
        if (!flipped_) {
            return *this;
        }
        else {
            std::move(active_.begin(), active_.end(), shadow_.begin());
            return ShadowDataPtr(shadow_, active_, !flipped_);
        }
    }
};


// template <typename Type, typename DataSet>
// inline
// typename std::enable_if<sizeof(Type) == 1, uint32_t>::type
// get_key(const DataSet& dset,
//         const typename DataSet::String& s, size_t depth) {
//     return dset.get_uint8(s, depth);
// }

// template <typename Type, typename DataSet>
// inline
// typename std::enable_if<sizeof(Type) == 4, uint32_t>::type
// get_key(const DataSet& dset,
//         const typename DataSet::String& s, size_t depth) {
//     return dset.get_uint32(s, depth);
// }

// template <typename Type, typename DataSet>
// inline
// typename std::enable_if<sizeof(Type) == 8, uint64_t>::type
// get_key(const DataSet& dset,
//         const typename DataSet::String& s, size_t depth) {
//     return dset.get_uint64(s, depth);
// }

} // namespace parallel_radixsort_detail
} // namespace tlx

#endif /* SHADOW_SET_H */
