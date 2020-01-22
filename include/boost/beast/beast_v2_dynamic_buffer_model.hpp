#pragma once

namespace boost {
namespace beast {

template<class Storage>
class beast_v2_dynamic_buffer_model
{
    using storage_type = Storage;

public:

    using mutable_buffers_type = typename storage_type::mutable_buffers_type;

    using const_buffers_type = typename storage_type::const_buffers_type;

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
    data(std::size_t pos, std::size_t n) const
    {
        return static_cast<storage_type const*>(storage_)->data(pos, n);
    }

    mutable_buffers_type
    data(std::size_t pos, std::size_t n)
    {
        return storage_->data(pos, n);
    }

    void
    grow(std::size_t n)
    {
        if (size() + n > max_size())
            throw std::length_error("grow");

        return storage_->grow(n);
    }

    void
    shrink(std::size_t n)
    {
        return storage_->shrink(n);
    }

    void
    consume(std::size_t n)
    {
        return storage_->consume(n);
    }

public:

    beast_v2_dynamic_buffer_model(storage_type &storage)
        : storage_(std::addressof(storage))
    {}

private:

    storage_type *storage_;
};

}
}