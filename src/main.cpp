#include "config.hpp"

#include "explain.hpp"
#include <iostream>
#include <boost/type_index.hpp>

namespace program {

using string_buffer_v1 =
net::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;

struct string_buffer_v2
{
    using mutable_buffers_type = net::mutable_buffers_1;
    using const_buffers_type = net::const_buffers_1;

    string_buffer_v2(
        std::string &store,
        std::size_t maximum_size = (std::numeric_limits<std::size_t>::max)())
        : store_(store)
        , maximum_size_(maximum_size)
    {}

    std::size_t
    capacity() const
    { return store_.capacity(); }

    void
    consume(
        std::size_t n)
    {
        store_.erase(0, n);
    }

    mutable_buffers_type
    data(
        std::size_t pos,
        std::size_t n)
    {
        return mutable_buffers_type(&store_[pos], n);
    }

    void
    grow(std::size_t n)
    {
        if (size() + n > max_size())
            throw std::length_error("grow");

        store_.resize(size() + n);
    }

    std::size_t
    max_size() const
    {
        return maximum_size_;
    }

    void
    shrink(std::size_t n)
    {
        if (n > size())
            n = size();

        store_.resize(size() - n);
    }

    std::size_t
    size() const
    {
        return store_.length();
    }

private:
    std::string &store_;
    std::size_t maximum_size_;
};

static_assert(sizeof(net::detail::size_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::max_size_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::capacity_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::data_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::consume_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::grow_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::shrink_memfn_helper<string_buffer_v2>(0)) != 1, "");
static_assert(sizeof(net::detail::const_buffers_type_typedef_helper<string_buffer_v2>(0)) == 1, "");
static_assert(sizeof(net::detail::mutable_buffers_type_typedef_helper<string_buffer_v2>(0)), "");
static_assert(net::is_dynamic_buffer_v2<string_buffer_v2>::value, "");

template<class T>
struct beast_v1_dynamic_buffer_test
{
    static auto match(...) -> std::false_type;
    template<class Alloc> auto match(beast::basic_flat_buffer<Alloc>*) -> std::true_type;

    static constexpr auto test() -> decltype(match((T*)0));
};

template<class T>
    struct is_beast_v1_dynamic_buffer : decltype(beast_v1_dynamic_buffer_test<T>::test()) {};

struct asio_v1_behaviour
{
};
struct asio_v2_behaviour
{
};
struct beast_v1_behaviour
{
};

template<class Candidate, class Enable = void>
struct select_dynamic_buffer_behaviour;

template<class Candidate>
struct select_dynamic_buffer_behaviour
    <
        Candidate,
        typename std::enable_if<
            net::is_dynamic_buffer_v1<Candidate>::value &&
            !is_beast_v1_dynamic_buffer<Candidate>::value
        >::type
    >
{
    using type = asio_v1_behaviour;
};

template<class Candidate>
struct select_dynamic_buffer_behaviour
    <
        Candidate,
        typename std::enable_if<
            !net::is_dynamic_buffer_v1<Candidate>::value &&
            net::is_dynamic_buffer_v2<Candidate>::value &&
            !is_beast_v1_dynamic_buffer<Candidate>::value
        >::type
    >
{
    using type = asio_v2_behaviour;
};

template<class Candidate>
struct select_dynamic_buffer_behaviour
    <
        Candidate,
        typename std::enable_if<
            is_beast_v1_dynamic_buffer<Candidate>::value
        >::type
    >
{
    using type = beast_v1_behaviour;
};

template<class Candidate>
using select_dynamic_buffer_behaviour_t =
typename select_dynamic_buffer_behaviour<Candidate>::type;

template
    <
        class DynamicBufferyThing,
        class Semantics = select_dynamic_buffer_behaviour_t<DynamicBufferyThing>
    >
struct dynamic_buffer_handle;

/// Handle to V1 dynamic buffer. We must take ownership of the buffer.
template<class BufferV1Type>
struct dynamic_buffer_handle<BufferV1Type, asio_v1_behaviour>
{
    using mutable_buffers_type = typename BufferV1Type::mutable_buffers_type;

    dynamic_buffer_handle(BufferV1Type dyn_buf)
        : dyn_buf_(std::make_shared<BufferV1Type>(std::move(dyn_buf)))
    {
    }

    auto
    prepare(std::size_t requested) -> mutable_buffers_type
    {
        auto buffers = dyn_buf_->prepare(requested);
        return buffers;
    }

    auto
    commit(std::size_t n) -> void
    {
        dyn_buf_->commit(n);
    }

    // other common ops here
    std::shared_ptr<BufferV1Type> dyn_buf_;
};

/// Handle to V1 dynamic buffer. We must not take ownership of the buffer.
template<class BufferV2Type>
struct dynamic_buffer_handle<BufferV2Type, asio_v2_behaviour>
{
    using mutable_buffers_type = typename BufferV2Type::mutable_buffers_type;

    struct impl_class
    {
        impl_class(BufferV2Type dyn_buf)
            : dyn_buf_(std::move(dyn_buf))
            , original_size_(dyn_buf.size())
            , prepared_space_(0)
        {}

        BufferV2Type dyn_buf_;
        std::size_t original_size_;
        std::size_t prepared_space_;
    };

    dynamic_buffer_handle(BufferV2Type dyn_buf)
        : impl_(std::make_shared<impl_class>(std::move(dyn_buf)))
    {}

    auto
    prepare(std::size_t n) -> mutable_buffers_type
    {
        impl_->original_size_ = impl_->dyn_buf_.size();
        impl_->prepared_space_ = n;
        impl_->dyn_buf_.grow(n);
        auto buffers =
            impl_->dyn_buf_.data(
                impl_->original_size_,
                impl_->prepared_space_);
        return buffers;
    }

    void
    commit(std::size_t n)
    {
        if (n < impl_->prepared_space_)
        {
            auto excess = impl_->prepared_space_ - n;
            impl_->dyn_buf_.shrink(excess);
        }
    }

    // other common ops here

private:
    using impl_type = std::shared_ptr<impl_class>;
    impl_type impl_;
};

/// Handle to V1 dynamic buffer. We must not take ownership of the buffer.
template<class BeastV1Type>
struct dynamic_buffer_handle<BeastV1Type, beast_v1_behaviour>
{
    using mutable_buffers_type = typename BeastV1Type::mutable_buffers_type;
    using const_buffers_type = typename BeastV1Type::const_buffers_type;

    dynamic_buffer_handle(BeastV1Type& dyn_buffer)
    : impl_(std::addressof(dyn_buffer))
    {}

    auto
    prepare(std::size_t n) -> mutable_buffers_type
    {
        return impl_->prepare(n);
    }

    void
    commit(std::size_t n)
    {
        return impl_->commit(n);
    }

    // other common ops here

private:
    using impl_type = BeastV1Type*;
    impl_type impl_;
};


template<class...Whatever>
std::size_t
dynamic_write_one(
    dynamic_buffer_handle<Whatever...> handle,
    net::const_buffers_1 source)
{
    auto buffers = handle.prepare(source.size());
    auto copied = buffer_copy(buffers, source);
    handle.commit(copied);
    return copied;
}

template<class...Whatever, class BufferSequence>
std::size_t
dynamic_write(
    dynamic_buffer_handle<Whatever...> handle,
    BufferSequence const &sources)
{
    auto total = std::size_t(0);
    for (auto buffer : sources)
        total += dynamic_write_one(handle, buffer);
    return total;
}


template<class Buffer, class...Strings>
void
test_write(
    Buffer &&buf,
    Strings &&...data)
{
    using buffer_handle_type = dynamic_buffer_handle<typename std::decay<Buffer>::type>;

    dynamic_write(buffer_handle_type(std::forward<Buffer>(buf)),
                  std::array<net::const_buffers_1, sizeof...(Strings)>{{
                                                                           net::const_buffers_1(net::buffer(data))...
                                                                       }});
}

template<class BufferType>
void print_assertion()
{
    auto type_name = boost::typeindex::type_id<BufferType>().pretty_name();

    std::cout << type_name << " is dynamic_buffer_v1? " << std::boolalpha
              << net::is_dynamic_buffer_v1<BufferType>::value << '\n';

    std::cout << type_name << " is dynamic_buffer_v2? " << std::boolalpha
              << net::is_dynamic_buffer_v2<BufferType>::value << '\n';

    std::cout << type_name << " is is_beast_v1_dynamic_buffer? " << std::boolalpha
              << is_beast_v1_dynamic_buffer<BufferType>::value << '\n';

    std::cout << type_name << " is dynamic_buffer? " << std::boolalpha
              << net::is_dynamic_buffer<BufferType>::value << '\n';

}

int
run()
{
    print_assertion<string_buffer_v1>();
    print_assertion<string_buffer_v2>();
    print_assertion<beast::flat_buffer>();

    auto d1 = std::string();
    test_write(string_buffer_v1(d1), std::string("Hello"), std::string(", "), std::string("World!"));

    std::cout << "result of v1 write: " << d1 << '\n';

    auto d2 = std::string();
    test_write(string_buffer_v2(d2), std::string("Hello"), std::string(", "), std::string("World!"));

    std::cout << "result of v2 write: " << d2 << '\n';

    auto fb = beast::flat_buffer();
    test_write(fb, std::string("Hello"), std::string(", "), std::string("World!"));
    std::cout << "result of beast v1 write: " << beast::buffers_to_string(fb.data()) << '\n';

    return 0;
}
}

int
main()
{
    try
    {
        return program::run();
    }
    catch (...)
    {
        std::cerr << program::explain() << std::endl;
        return 127;
    }
}