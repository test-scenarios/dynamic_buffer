#pragma once

#include <boost/asio.hpp>
#undef BOOST_ASIO_BUFFER_HPP
namespace boost {
namespace asio {
namespace v2 {
#define BOOST_ASIO_NO_DYNAMIC_BUFFER_V1

namespace boost
{
    using ::boost::array;
    namespace asio
    {
    namespace detail
    {
    using ::boost::asio::detail::throw_exception;
    using ::boost::asio::detail::addressof;
    using ::boost::asio::detail::is_buffer_sequence;
    using ::boost::asio::detail::is_dynamic_buffer_v2;

    }
    }
}

#include <boost/asio/buffer.hpp>

}
}
}

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace program
{
    using string_buffer_v1 = boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;
    using string_buffer_v2 = boost::asio::v2::boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;
    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace ssl = boost::asio::ssl;
}