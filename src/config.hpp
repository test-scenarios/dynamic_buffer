#pragma once

#include <boost/asio.hpp>

// The following hack re-includes the boost/asio/buffer.hpp file within the namespace
// boost::asio::v2
// This will allow us to construct both v1 and v2 forms of the dynamic buffer specialisations
#undef BOOST_ASIO_BUFFER_HPP
namespace boost {
namespace asio {
    namespace v2 {
    namespace boost {
        using ::boost::array;
        namespace asio {
        namespace detail {
            using ::boost::asio::detail::throw_exception;
            using ::boost::asio::detail::addressof;
            using ::boost::asio::detail::is_buffer_sequence;
            using ::boost::asio::detail::is_dynamic_buffer_v2;
        }
        }
    }
#define BOOST_ASIO_NO_DYNAMIC_BUFFER_V1
#include <boost/asio/buffer.hpp>
}
}
}

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace program {
namespace net = boost::asio;
namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
}