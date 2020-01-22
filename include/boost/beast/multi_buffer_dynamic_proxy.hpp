#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/optional/optional.hpp>
#include <type_traits>

namespace boost {
namespace beast {

class multi_buffer_dynamic_proxy_base
{
public:

    using mutable_buffers_type = std::vector<net::mutable_buffer>;

    using const_buffers_type = std::vector<net::const_buffer>;

protected:

    // internal utilities
    template<class BufferSequence>
    static void
    trim(
        BufferSequence& sequence,
        std::size_t pos,
        std::size_t n)
    {
        auto current = sequence.begin();
        // trim up to pos
        for (; pos && current != sequence.end();)
        {
            if (current->size() <= pos)
            {
                pos -= current->size();
                current = sequence.erase(current);
            }
            else
            {
                *current += pos;
                pos = 0;
            }
        }

        // current_buffer now contains the first buffer that contains data we are interested in

        // insert up to n
        for ( ; current != sequence.end() ; )
        {
            if (current->size() < n)
            {
                ++current;
            }
            else
            {
                using buffer_type = typename std::decay<decltype(*current)>::type;
                *current = buffer_type{current->data(), n};
                ++current;
                n = 0;
            }
        }

        sequence.erase(current, sequence.end());
    }
};

template<class Allocator>
class multi_buffer_dynamic_proxy : public multi_buffer_dynamic_proxy_base
{
    using storage_type = basic_multi_buffer<Allocator>;

    storage_type *storage_;

    // optionally the region that was last prepared
    optional<typename storage_type::mutable_buffers_type> prepared_region_;

public:
    // constructor
    multi_buffer_dynamic_proxy(storage_type &store)
        : storage_(std::addressof(store))
    {}

public:
    // implement the asio v2 dynamic_buffer interface

    std::size_t
    size() const
    {
        return storage_->size();
    }

    std::size_t
    max_size() const
    {
        return storage_->max_size();
    }

    std::size_t
    capacity() const
    {
        return storage_->capacity();
    }

    const_buffers_type
    data(
        std::size_t pos,
        std::size_t n) const
    {
        auto result = const_buffers_type();
        for(auto&& buf : storage_->data())
        {
            if (buf.size())
            {
                result.emplace_back(buf.data(), buf.size());
            }
        }

        if (prepared_region_.has_value())
        {
            for(auto&& buf : *prepared_region_)
            {
                if (buf.size())
                {
                    result.emplace_back(buf.data(), buf.size());
                }
            }
        }

        trim(result, pos, n);

        return result;
    }

    auto
    data(
        std::size_t pos,
        std::size_t n)
    -> mutable_buffers_type
    {
        auto result = mutable_buffers_type();
        for(auto&& buf : storage_->data())
        {
            if (buf.size())
            {
                result.emplace_back(buf.data(), buf.size());
            }
        }

        if (prepared_region_.has_value())
        {
            for(auto&& buf : *prepared_region_)
            {
                if (buf.size())
                {
                    result.emplace_back(buf.data(), buf.size());
                }
            }
        }

        trim(result, pos, n);

        return result;
    }

    void
    grow(std::size_t n)
    {
        if (size() + n > max_size())
            throw std::length_error("grow");

        BOOST_ASSERT(!prepared_region_.has_value());

        prepared_region_.emplace(storage_->prepare(n));
    }

    void
    shrink(std::size_t n)
    {
        BOOST_ASSERT(prepared_region_.has_value());
        auto commit_size = net::buffer_size(*prepared_region_) - n;
        storage_->commit(commit_size);
        prepared_region_.reset();
    }

    void
    consume(std::size_t n)
    {
        return storage_->consume(n);
    }

private:


};



template<class Allocator>
auto
dynamic_buffer(basic_multi_buffer <Allocator> &storage)
-> multi_buffer_dynamic_proxy<Allocator>
{
    static_assert(net::is_dynamic_buffer_v2<multi_buffer_dynamic_proxy<Allocator>>::value, "");
    return multi_buffer_dynamic_proxy<Allocator>(storage);
}


}
}