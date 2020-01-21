#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>

namespace boost {
namespace beast {

/// Opaque storage type for resizable buffer storage in layout of contiguous bytes
struct flat_storage
{
// interface
    using mutable_buffers_type = net::mutable_buffer;
    using const_buffers_type = net::const_buffer;

    mutable_buffers_type
    prepare_input(std::size_t n)
    {
        if (capacity_ - size_ < n)
        {
            auto new_cap = size_ + n;
            if (new_cap > max_capacity_)
                boost::throw_exception(std::length_error("out of space"));

            auto oldp = store_.release();
            auto newp = reinterpret_cast<char *>(std::realloc(oldp, new_cap));
            if (newp)
            {
                store_.reset(newp);
                capacity_ = new_cap;
            }
            else
            {
                store_.reset(oldp);
                throw std::bad_alloc();
            }
        }
        auto result = mutable_buffers_type(store_.get() + size_, n);
        size_ += n;
        return result;
    }

    void
    consume(std::size_t n)
    {
        n = std::min(n, size_);
        std::memcpy(store_.get(), store_.get() + n, size_ - n);
        size_ -= n;
    }

    void
    dispose_input(std::size_t n)
    {
        size_ -= std::min(n, size_);
    }

    const_buffers_type
    data() const
    {
        return const_buffers_type(store_.get(), size_);
    }

// constructors
public:
    flat_storage(std::size_t limit = std::numeric_limits<std::size_t>::max())
        : size_(0)
        , capacity_(0)
        , max_capacity_(limit)
        , store_(nullptr)
    {}

private:

    std::size_t
    max_capacity() const
    {
        return max_capacity_;
    }

    std::size_t size_;
    std::size_t capacity_;
    std::size_t max_capacity_;

    struct deleter
    {
        void
        operator()(char *p) const
        {
            std::free(p);
        }
    };

    std::unique_ptr<char, deleter> store_;
};

struct flat_storage_dynamic_buffer
    : beast_v2_dynamic_buffer_model<flat_storage>
{
    using beast_v2_dynamic_buffer_model::beast_v2_dynamic_buffer_model;
};

auto dynamic_buffer(flat_storage& storage)
-> flat_storage_dynamic_buffer
{
    return { storage };
}


}
}