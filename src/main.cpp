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

template<class Buffer>
void test_write(Buffer&& buf, std::string const& data)
{
    using buffer_handle_type = dynamic_buffer_handle<typename std::decay<Buffer>::type >;

    auto mybuf = buffer_handle_type(std::forward<Buffer>(buf));

    mybuf.write(net::buffer(data));
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
    test_write(string_buffer_v1(d1), "hello");

    std::cout << "result of v1 write: " << d1 << '\n';

    auto d2 = std::string();
    auto db2 = string_buffer_v2(d2);
    test_write(db2, "hello");

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