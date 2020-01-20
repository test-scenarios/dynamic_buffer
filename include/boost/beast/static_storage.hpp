#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>

namespace boost {
namespace beast {

/// Opaque storage type for non-extentable buffer storage in layout of contiguous bytes
template<std::size_t Capacity>
struct static_storage;

template<class IntegralCapacity>
struct static_storage_dynamic_buffer;

template<std::size_t Capacity>
struct static_storage
{
// interface
    using mutable_buffers_type = net::mutable_buffer;
    using const_buffers_type = net::const_buffer;

    auto
    prepare_input(std::size_t n) -> mutable_buffers_type
    {
        if (max_capacity() - size_ < n)
            boost::throw_exception(std::length_error("out of space"));
        auto result = mutable_buffers_type(store_ + size_, n);
        size_ += n;
        return result;
    }

    void consume(std::size_t n)
    {
        n = std::min(n, size_);
        std::memcpy(store_, store_ + n, size_ - n);
        size_ -= n;
    }

    void
    dispose_input(std::size_t n)
    {
        size_ -= std::min(n, size_);
    }

    const_buffers_type data() const
    {
        return const_buffers_type (store_, size_);
    }

    constexpr static
    std::size_t
    max_capacity()
    {
        return Capacity;
    }


// constructors
public:
    static_storage()
        : size_(0)
    {}

private:
    std::size_t size_;
    char store_[Capacity];
};

template<class Storage>
struct static_storage_dynamic_buffer;

template<std::size_t Capacity>
struct static_storage_dynamic_buffer<std::integral_constant<std::size_t, Capacity>>
: beast_v2_dynamic_buffer_model<static_storage_dynamic_buffer<std::integral_constant<std::size_t, Capacity>>>
{
    using base_class = beast_v2_dynamic_buffer_model<static_storage_dynamic_buffer<std::integral_constant<std::size_t, Capacity>>>;
    using storage_type = typename base_class::storage_type;

    using base_class::base_class;

};

}
}