#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>

namespace boost{
namespace beast {

/// Opaque storage type for resizable buffer storage in circular layout
struct circular_storage;
struct circular_storage_dynamic_buffer;
template<> struct is_beast_v2_dynamic_buffer<circular_storage_dynamic_buffer>: std::true_type {};

}}