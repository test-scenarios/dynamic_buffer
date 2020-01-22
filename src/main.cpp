#include "config.hpp"

#include "explain.hpp"
#include <iostream>
#include <boost/type_index.hpp>

#if !NO_FLAT_STORAGE
#include <boost/beast/flat_storage.hpp>
#endif
#if !NO_MULTI_STORAGE
#include <boost/beast/multi_storage.hpp>
#endif
#if !NO_CIRCULAR_STORAGE
//#include <boost/beast/circular_storage.hpp>
#endif
#include <boost/beast/static_storage.hpp>

#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>

struct make_static
{
    enum
        : std::size_t
    {
        max_capacity = 16
    };

    auto
    operator()() const -> boost::beast::static_storage<max_capacity>
    {
        return {};
    }
};

#if !NO_FLAT_STORAGE
struct make_flat
{
    enum
        : std::size_t
    {
        max_capacity = 16
    };

    auto
    operator()() const -> boost::beast::flat_storage
    {
        return boost::beast::flat_storage(max_capacity);
    }
};
#endif

#if !NO_CIRCULAR_STORAGE
struct make_circular
{
    enum
        : std::size_t
    {
        max_capacity = 16
    };

    auto
    operator()() const -> boost::beast::circular_storage
    {
        return boost::beast::circular_storage(max_capacity);
    }
};
#endif

#if !NO_MULTI_STORAGE
struct make_multi
{
    enum
        : std::size_t
    {
        max_capacity = 16
    };

    auto
    operator()() const -> boost::beast::multi_storage
    {
        return boost::beast::multi_storage(max_capacity);
    }
};
#endif

using test_list = std::tuple<
    make_static
#if !NO_FLAT_STORAGE
    , make_flat
#endif
#if !NO_CIRCULAR_STORAGE
    , make_circular
#endif
#if !NO_MULTI_STORAGE
    , make_multi
#endif
    >;

TEMPLATE_LIST_TEST_CASE("beast v2 storage types", "", test_list)
{
    using namespace boost::beast;

    auto storage = TestType()();

    REQUIRE(net::is_dynamic_buffer_v2<decltype(storage)>::value == false);
    REQUIRE(net::is_dynamic_buffer<decltype(storage)>::value == false);
}

TEMPLATE_LIST_TEST_CASE("beast v2 dynamic buffer types", "", test_list)
{
    using namespace boost::beast;

    auto storage = TestType()();
    auto dyn_buf = dynamic_buffer(storage);
    CHECK(dyn_buf.size() < dyn_buf.max_size());
    CHECK(dyn_buf.size() == 0);
    CHECK(net::buffer_size(dyn_buf.data(0, dyn_buf.size())) == 0);

    auto do_insert = [&dyn_buf](net::const_buffer source)
    {
        auto start = dyn_buf.size();
        auto len = source.size();
        dyn_buf.grow(len);
        auto insert_region = dyn_buf.data(start, len);
        CHECK(net::buffer_size(insert_region) == len);
        auto copied = net::buffer_copy(insert_region, source);
        CHECK(copied == len);
    };

    do_insert(net::buffer(std::string("0123456789")));
    dyn_buf.shrink(1);
    auto output_region = dyn_buf.data(0, dyn_buf.size());
    REQUIRE(net::buffer_size(output_region) == 9);
    REQUIRE(buffers_to_string(output_region) == "012345678");

    do_insert(net::buffer(std::string("9abcdef")));
    dyn_buf.shrink(0);
    output_region = dyn_buf.data(0, dyn_buf.size());
    REQUIRE(net::buffer_size(output_region) == 16);
    REQUIRE(buffers_to_string(output_region) == "0123456789abcdef");

    REQUIRE_THROWS_AS(dyn_buf.grow(1), std::length_error);

    dyn_buf.consume(10);
    output_region = dyn_buf.data(0, dyn_buf.size());
    REQUIRE(net::buffer_size(output_region) == 6);
    REQUIRE(buffers_to_string(output_region) == "abcdef");

    dyn_buf.consume(10);
    output_region = dyn_buf.data(0, dyn_buf.size());
    REQUIRE(net::buffer_size(output_region) == 0);
}


int
main(
    int argc,
    char **argv)
{
    try
    {
        int result = Catch::Session().run(argc, argv);
        return result;
    }
    catch (...)
    {
        std::cerr << program::explain() << std::endl;
        return 127;
    }
}