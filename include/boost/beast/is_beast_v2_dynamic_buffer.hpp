#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <type_traits>

namespace boost {
namespace beast {

template<class T, class = void>
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
    prepare(std::size_t n) -> mutable_buffers_type
    {
        return storage_->prepare(n);
    }

    void
    dispose_input(std::size_t n)
    {
        return storage_->dispose_input(n);
    }

    const_buffers_type
    data() const
    {
        return storage_->data();
    }

    void
    consume(std::size_t n)
    {
        return storage_->consume(n);
    }

    std::size_t
    size() const
    {
        return net::buffer_size(storage_->data());
    }

    std::size_t
    max_size() const
    {
        return storage_->max_size();
    }

public:

    beast_v2_dynamic_buffer_model(storage_type &storage)
        : storage_(std::addressof(storage))
    {}

private:

    storage_type *storage_;
};

template<class T, class...Types>
constexpr std::true_type
check_is_beast_v2_dynamic_buffer_model(beast_v2_dynamic_buffer_model<Types...> &&)
{ return {}; }

template<class T>
constexpr std::false_type
check_is_beast_v2_dynamic_buffer_model(...)
{ return {}; }

template<typename T>
using is_beast_v2_dynamic_buffer_model = decltype(check_is_beast_v2_dynamic_buffer_model<T>(std::declval<T>()));

template<class Derived>
struct is_beast_v2_dynamic_buffer<Derived, typename std::enable_if<is_beast_v2_dynamic_buffer_model<Derived>::value>::type>
    : std::true_type
{
};


//template<class Storage>
//struct is_beast_v2_dynamic_buffer<beast_v2_dynamic_buffer_model<Storage>>
//    : std::true_type
//{
//};



}
}