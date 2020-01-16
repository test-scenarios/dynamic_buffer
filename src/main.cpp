#include "config.hpp"

#include "explain.hpp"
#include <iostream>

namespace program {

using string_buffer_v1 =
net::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;

using string_buffer_v2 =
net::v2::boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;

struct use_dynamic_buffer_v1_semantics {};
struct use_dynamic_buffer_v2_semantics {};

template<class Candidate, class Enable = void>
struct select_dynamic_buffer_semantics;

template <class Candidate>
struct select_dynamic_buffer_semantics
<
    Candidate,
    typename std::enable_if<
        net::is_dynamic_buffer_v1<Candidate>::value
    >::type
>
{
    using type = use_dynamic_buffer_v1_semantics;
};

template <class Candidate>
struct select_dynamic_buffer_semantics
    <
        Candidate,
        typename std::enable_if<
            ! net::is_dynamic_buffer_v1<Candidate>::value &&
              net::is_dynamic_buffer_v2<Candidate>::value
        >::type
    >
{
    using type = use_dynamic_buffer_v2_semantics;
};

template <class Candidate>
using select_dynamic_buffer_semantics_t =
    typename select_dynamic_buffer_semantics<Candidate>::type;

template
<
    class DynamicBufferyThing,
    class Semantics = select_dynamic_buffer_semantics_t<DynamicBufferyThing>
>
struct dynamic_buffer_handle;

/// Handle to V1 dynamic buffer. We must take ownership of the buffer.
template<class BufferV1Type>
struct dynamic_buffer_handle<BufferV1Type, use_dynamic_buffer_v1_semantics>
{
    dynamic_buffer_handle(BufferV1Type dyn_buf)
    : dyn_buf_(std::make_shared<BufferV1Type>(std::move(dyn_buf)))
    {
    }

    std::size_t
    write(net::const_buffer source)
    {
        auto target = dyn_buf_->prepare(source.size());
        auto copied = net::buffer_copy(target, source);
        dyn_buf_->commit(copied);
        return copied;
    }

    // other common ops here
    std::shared_ptr<BufferV1Type> dyn_buf_;
};

/// Handle to V1 dynamic buffer. We must not take ownership of the buffer.
template<class BufferV2Type>
struct dynamic_buffer_handle<BufferV2Type, use_dynamic_buffer_v2_semantics>
{
    dynamic_buffer_handle(BufferV2Type& dyn_buf)
        : dyn_buf_(std::addressof(dyn_buf))
    {}

    dynamic_buffer_handle(BufferV2Type&& dyn_buf)
        : dyn_buf_(std::addressof(dyn_buf))
    {}

    std::size_t
    write(net::const_buffer source_v1)
    {
        auto source = net::v2::boost::asio::const_buffer(source_v1.data(), source_v1.size());
        auto current_size = dyn_buf_->size();
        dyn_buf_->grow(source.size());
        auto target = dyn_buf_->data(current_size, source.size());
        auto copied = buffer_copy(target, source);
        return copied;
    }

    // other common ops here

    BufferV2Type* dyn_buf_;
};

template <class...Whatever>
std::size_t
dynamic_write_one(dynamic_buffer_handle<Whatever...> handle, net::const_buffers_1 source)
{
    return handle.write(source);
}

template <class...Whatever, class BufferSequence>
std::size_t
dynamic_write(dynamic_buffer_handle<Whatever...> handle, BufferSequence const& sources)
{
    auto total = std::size_t(0);
    for(auto buffer : sources)
        total += dynamic_write_one(handle, buffer);
    return total;
}


template<class Buffer, class...Strings>
void test_write(Buffer&& buf, Strings&&...data)
{
    using buffer_handle_type = dynamic_buffer_handle<typename std::decay<Buffer>::type >;

    dynamic_write(buffer_handle_type(std::forward<Buffer>(buf)),
        std::array<net::const_buffers_1, sizeof...(Strings)>{{
                 net::const_buffers_1(net::buffer(data))...
    }});
}

int
run()
{
//        std::string data;
//        auto sv1 = string_buffer_v1(data);
//        auto sv2 = string_buffer_v2(data);

    std::cout << "string_buffer_v1 is dynamic_buffer_v1? " << std::boolalpha
              << net::is_dynamic_buffer_v1<string_buffer_v1>::value << '\n';
    std::cout << "string_buffer_v1 is dynamic_buffer_v2? " << std::boolalpha
              << net::is_dynamic_buffer_v2<string_buffer_v1>::value << '\n';
    std::cout << "string_buffer_v1 is dynamic_buffer? " << std::boolalpha
              << net::is_dynamic_buffer<string_buffer_v1>::value << '\n';

    std::cout << "string_buffer_v2 is dynamic_buffer_v1? " << std::boolalpha
              << net::is_dynamic_buffer_v1<string_buffer_v2>::value << '\n';
    std::cout << "string_buffer_v2 is dynamic_buffer_v2? " << std::boolalpha
              << net::is_dynamic_buffer_v2<string_buffer_v2>::value << '\n';
    std::cout << "string_buffer_v2 is dynamic_buffer? " << std::boolalpha
              << net::is_dynamic_buffer<string_buffer_v2>::value << '\n';

    auto d1 = std::string();
    test_write(string_buffer_v1(d1), std::string("Hello"), std::string(", "), std::string("World!"));

    std::cout << "result of v1 write: " << d1 << '\n';

    auto d2 = std::string();
    test_write(string_buffer_v2(d2), std::string("Hello"), std::string(", "), std::string("World!"));

    std::cout << "result of v2 write: " << d2 << '\n';

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