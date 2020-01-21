#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>

namespace boost {
namespace beast {

/// Opaque storage type for resizable buffer storage in layout of chunks of bytes
struct multi_storage
{
    // for now, implement in terms of multi_buffer. This can be optimised later
    using store_type = multi_buffer;

    using mutable_buffers_type = store_type::mutable_buffers_type;
    using const_buffers_type = store_type::const_buffers_type;

    mutable_buffers_type
    prepare(std::size_t n)
    {
        auto result = store_.prepare(n);
        prepared_ += n;
        return result;
    }

    void
    consume(std::size_t n)
    {
        return store_.consume(n);
    }

    void
    dispose_input(std::size_t n)
    {
        n = std::min(prepared_, n);
        if (prepared_)
        {
            store_.commit(prepared_ - n);
        }
        prepared_ = 0;
    }

    const_buffers_type
    data() const
    {
        return store_.data();
    }

    std::size_t
    max_size() const
    {
        return store_.max_size();
    }

public:

    multi_storage()
        : store_()
    {}

    multi_storage(std::size_t limit)
        : store_(limit)
    {}

private:
    store_type store_;
    std::size_t prepared_ = 0;
};

struct multi_storage_dynamic_buffer
    : beast_v2_dynamic_buffer_model<multi_storage>
{
    using beast_v2_dynamic_buffer_model::beast_v2_dynamic_buffer_model;
};

inline auto
dynamic_buffer(multi_storage &storage)
-> multi_storage_dynamic_buffer
{
    return {storage};
}

}
}