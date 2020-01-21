#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>
#include <array>

namespace boost {
namespace beast {

template<class BufferType>
struct basic_upto2_buffers
    {
    using storage_type = std::array<BufferType, 2>;
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    const_iterator
    begin() const
    { return store_.data(); }

    iterator
    begin()
    { return store_.data(); }

    const_iterator
    end() const
    { return begin() + size_; }

    iterator
    end()
    { return begin() + size_; }

public:
    explicit basic_upto2_buffers(
        BufferType b1 = {},
        BufferType b2 = {})
        : store_()
        , size_(0)
    {
        maybe_append(b1);
        maybe_append(b2);
    }

private:

    void
    maybe_append(BufferType b)
    {
        if (b.size() == 0)
            return;

        store_[size_++] = b;
    }

private:
    storage_type store_;
    std::size_t size_;
};

using mutable_upto2_buffers = basic_upto2_buffers<net::mutable_buffer>;
using const_upto2_buffers = basic_upto2_buffers<net::const_buffer>;


/// Opaque storage type for resizable buffer storage in circular layout
struct circular_storage
{
// interface
    using mutable_buffers_type = mutable_upto2_buffers;
    using const_buffers_type = const_upto2_buffers;

    mutable_buffers_type
    prepare(std::size_t n)
    {
        if (n > (capacity_ - size_))
        {
            boost::throw_exception(std::length_error("out of space"));
        }

        auto here = input_end_;
        input_end_ = advance_ptr(input_end_, n);
        size_ += n;

        return region(here, input_end_);
    }

    void
    consume(std::size_t n)
    {
        n = std::min(size_, n);
        output_start_ = advance_ptr(output_start_, n);
        size_ -= n;
    }

    void
    dispose_input(std::size_t n)
    {
        n = std::min(n, size_);
        size_ -= n;
        input_end_ = retreat_ptr(input_end_, n);
    }

    const_buffers_type
    data() const
    {
        auto result = size_
                      ? region(output_start_, input_end_)
                      : const_buffers_type();
        return result;
    }

    mutable_buffers_type
    data()
    {
        auto result = size_
                      ? region(output_start_, input_end_)
                      : mutable_buffers_type();
        return result;
    }

    std::size_t
    max_size() const
    {
        return capacity_;
    }

// constructors
public:
    circular_storage(std::size_t limit)
        : capacity_(limit)
        , size_(0)
        , store_(allocate(limit))
        , output_start_(store_.get())
        , input_end_(store_.get())
    {}

private:

    using pointer = char *;

    mutable_buffers_type
    region(
        pointer from,
        pointer to)
    {
        if (to <= from)
        {
            return mutable_buffers_type(
                asio::mutable_buffer(from, std::distance(from, end_store())),
                asio::mutable_buffer(begin_store(), std::distance(begin_store(), to))
            );
        }
        else
        {
            return mutable_buffers_type(
                asio::mutable_buffer(from, std::distance(from, to))
            );
        }
    }

    const_buffers_type
    region(
        pointer from,
        pointer to) const
    {
        if (to <= from)
        {
            return const_buffers_type(
                asio::const_buffer(from, std::distance(from, end_store())),
                asio::const_buffer(begin_store(), std::distance(begin_store(), to))
            );
        }
        else
        {
            return const_buffers_type(
                asio::const_buffer(from, std::distance(from, to))
            );
        }
    }

    // pre: there must be sufficient unused space
    pointer
    advance_ptr(
        pointer p,
        std::size_t n)
    {
        auto dist_to_end = std::size_t(std::distance(p, end_store()));
        if (dist_to_end <= n)
        {
            p = begin_store();
            n -= dist_to_end;
        }
        p += n;
        return p;
    }

    pointer
    retreat_ptr(
        pointer p,
        std::size_t n)
    {
        auto dist_from_start = std::size_t(std::distance(begin_store(), p));
        if (dist_from_start < n)
        {
            p = end_store();
            n -= dist_from_start;
        }
        p -= n;
        return p;
    }

    static
    pointer
    allocate(std::size_t n)
    {
        return reinterpret_cast<pointer>(std::malloc(n));
    }

    pointer
    begin_store() const
    {
        return store_.get();
    }

    pointer
    end_store() const
    {
        return begin_store() + capacity_;
    }


    struct deleter
    {
        void
        operator()(pointer p) const
        {
            std::free(p);
        }
    };

    std::size_t capacity_;
    std::size_t size_;          // size of used space. Used to avoid complex ptr atrithmetic
    std::unique_ptr<char, deleter> store_;
    pointer output_start_;
    pointer input_end_;

};

struct circular_storage_dynamic_buffer
    : beast_v2_dynamic_buffer_model<circular_storage>
{
    using beast_v2_dynamic_buffer_model::beast_v2_dynamic_buffer_model;
};

inline auto
dynamic_buffer(circular_storage &storage)
-> circular_storage_dynamic_buffer
{
    return {storage};
}


}
}