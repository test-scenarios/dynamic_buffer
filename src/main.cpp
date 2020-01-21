#include "config.hpp"

#include "explain.hpp"
#include <iostream>
#include <boost/type_index.hpp>

#include <boost/beast/flat_storage.hpp>
#include <boost/beast/multi_storage.hpp>
#include <boost/beast/circular_storage.hpp>
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

TEMPLATE_TEST_CASE("beast v2 storage types", "", make_static, make_flat, make_circular, make_multi)
{
    using namespace boost::beast;

    auto storage = TestType()();

    CHECK(net::buffer_size(storage.data()) == 0);

    auto insert_region = storage.prepare(10);
    CHECK(net::buffer_size(insert_region) == 10);
    net::buffer_copy(insert_region, net::buffer(std::string("0123456789")));
    storage.dispose_input(1);
    auto output_region = storage.data();
    REQUIRE(net::buffer_size(output_region) == 9);
    REQUIRE(buffers_to_string(output_region) == "012345678");

    insert_region = storage.prepare(7);
    CHECK(net::buffer_size(insert_region) == 7);
    net::buffer_copy(insert_region, net::buffer(std::string("9abcdef")));
    storage.dispose_input(0);
    output_region = storage.data();
    REQUIRE(net::buffer_size(output_region) == 16);
    REQUIRE(buffers_to_string(output_region) == "0123456789abcdef");

    REQUIRE_THROWS_AS(storage.prepare(1), std::length_error);

    storage.consume(10);
    output_region = storage.data();
    REQUIRE(net::buffer_size(output_region) == 6);
    REQUIRE(buffers_to_string(output_region) == "abcdef");

    storage.consume(10);
    output_region = storage.data();
    REQUIRE(net::buffer_size(output_region) == 0);
}

TEMPLATE_TEST_CASE("beast v2 dynamic buffer types", "", make_static, make_flat, make_circular, make_multi)
{
    using namespace boost::beast;

    auto storage = TestType()();
    auto dyn_buf = dynamic_buffer(storage);
    CHECK(dyn_buf.size() <= dyn_buf.max_size());
    CHECK(net::buffer_size(dyn_buf.data()) == 0);

    auto insert_region = dyn_buf.prepare(10);
    CHECK(net::buffer_size(insert_region) == 10);
    net::buffer_copy(insert_region, net::buffer(std::string("0123456789")));
    dyn_buf.dispose_input(1);
    auto output_region = dyn_buf.data();
    REQUIRE(net::buffer_size(output_region) == 9);
    REQUIRE(buffers_to_string(output_region) == "012345678");

    insert_region = dyn_buf.prepare(7);
    CHECK(net::buffer_size(insert_region) == 7);
    net::buffer_copy(insert_region, net::buffer(std::string("9abcdef")));
    dyn_buf.dispose_input(0);
    output_region = dyn_buf.data();
    REQUIRE(net::buffer_size(output_region) == 16);
    REQUIRE(buffers_to_string(output_region) == "0123456789abcdef");

    REQUIRE_THROWS_AS(dyn_buf.prepare(1), std::length_error);

    dyn_buf.consume(10);
    output_region = dyn_buf.data();
    REQUIRE(net::buffer_size(output_region) == 6);
    REQUIRE(buffers_to_string(output_region) == "abcdef");

    dyn_buf.consume(10);
    output_region = dyn_buf.data();
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