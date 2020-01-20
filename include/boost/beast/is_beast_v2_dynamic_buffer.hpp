#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <type_traits>

namespace boost {
namespace beast {

template<class T>
struct is_beast_v2_dynamic_buffer
    : std::false_type
{
};

template<class Storage>
struct beast_v2_dynamic_buffer_model
{
    using mutable_buffers_type = typename Storage::mutable_buffers_type;
    using const_buffers_type = typename Storage::const_buffers_type;
    using storage_type = Storage;

    auto
    prepare_input(std::size_t n) -> mutable_buffers_type
    {
        return storage_->prepare_input(n);
    }

    void
    dispose_input(std::size_t n)
    {
        return storage_->dispose_input(n);
    }

    const_buffers_type data() const
    {
        return storage_->data();
    }

    void consume(std::size_t n)
    {
        return storage_.consume(n);
    }

    std::size_t
    max_capacity() const
    {
        return storage_->max_capacity();
    }

public:

    beast_v2_dynamic_buffer_model(storage_type & storage)
    : storage_(std::addressof(storage))
    {}

private:

    storage_type *storage_;
};

template<class Storage>
struct is_beast_v2_dynamic_buffer<beast_v2_dynamic_buffer_model<Storage>>
    : std::true_type
{
};

}
}