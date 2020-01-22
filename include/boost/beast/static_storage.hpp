#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/beast_v2_dynamic_buffer_model.hpp>

namespace boost {
namespace beast {

/// Opaque storage type for non-extentable buffer storage in layout of contiguous bytes
template<std::size_t Capacity>
class static_storage;

template<class IntegralCapacity>
struct static_storage_dynamic_buffer;

template<std::size_t Capacity>
class static_storage
{
    std::size_t size_;
    char store_[Capacity];

    using this_class = static_storage<Capacity>;

private:
    // internal dynamic buffer interface
    using mutable_buffers_type = net::mutable_buffer;
    using const_buffers_type = net::const_buffer;

    std::size_t
    size() const
    {
        return size_;
    }

    constexpr static
    std::size_t
    max_size()
    {
        return Capacity;
    }

    constexpr static
    std::size_t
    capacity()
    {
        return Capacity;
    }

    const_buffers_type
    data(std::size_t pos, std::size_t n) const
    {
        BOOST_ASSERT(pos < size() || n == 0);
        BOOST_ASSERT(n + pos <= size());
        return const_buffers_type(store_ + pos, n);
    }

    mutable_buffers_type
    data(std::size_t pos, std::size_t n)
    {
        BOOST_ASSERT(pos < size() || n == 0);
        BOOST_ASSERT(n + pos <= size());
        return mutable_buffers_type(store_ + pos, n);
    }

    void
    grow(std::size_t n)
    {
        if (max_size() - size_ < n)
            boost::throw_exception(std::length_error("prepare"));

        size_ += n;
    }

    void
    shrink(std::size_t n)
    {
        size_ -= std::min(n, size_);
    }

    void
    consume(std::size_t n)
    {
        n = std::min(n, size_);
        std::memcpy(store_, store_ + n, size_ - n);
        size_ -= n;
    }

    friend beast_v2_dynamic_buffer_model<this_class>;

// constructors
public:
    static_storage()
        : size_(0)
    {}
};

template<class Storage>
struct static_storage_dynamic_buffer;

template<std::size_t Capacity>
struct static_storage_dynamic_buffer<std::integral_constant<std::size_t, Capacity>>
    : beast_v2_dynamic_buffer_model<static_storage<Capacity>>
{
    using base_class = beast_v2_dynamic_buffer_model<static_storage<Capacity>>;

    using base_class::base_class;
};

template<std::size_t Capacity>
auto
dynamic_buffer(static_storage<Capacity> &storage)
-> static_storage_dynamic_buffer<std::integral_constant<std::size_t, Capacity>>
{
    return {storage};
}


}
}