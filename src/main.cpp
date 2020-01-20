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
    enum : std::size_t { max_capacity = 16 };

    auto
    operator()() const -> boost::beast::static_storage<max_capacity>
    {
        return {};
    }
};

struct make_flat
{
    enum : std::size_t { max_capacity = 16 };

    auto
    operator()() const -> boost::beast::flat_storage
    {
        return boost::beast::flat_storage(max_capacity);
    }
};

TEMPLATE_TEST_CASE("beast v2 storage types", "", make_static, make_flat)
{
    using namespace boost::beast;

    auto storage = TestType()();

    CHECK(storage.data().size() == 0);
    CHECK(storage.max_capacity() == TestType::max_capacity);

    auto insert_region = storage.prepare_input(10);
    CHECK(insert_region.size() == 10);
    net::buffer_copy(insert_region, net::buffer(std::string("0123456789")));
    storage.dispose_input(1);
    auto output_region = storage.data();
    REQUIRE(output_region.size() == 9);
    REQUIRE(string_view(reinterpret_cast<const char*>(output_region.data()), output_region.size()) == "012345678");

    insert_region = storage.prepare_input(7);
    CHECK(insert_region.size() == 7);
    net::buffer_copy(insert_region, net::buffer(std::string("9abcdef")));
    storage.dispose_input(0);
    output_region = storage.data();
    REQUIRE(output_region.size() == 16);
    REQUIRE(string_view(reinterpret_cast<const char*>(output_region.data()), output_region.size()) == "0123456789abcdef");

    REQUIRE_THROWS_AS(storage.prepare_input(1), std::length_error);

    storage.consume(10);
    output_region = storage.data();
    REQUIRE(output_region.size() == 6);
    REQUIRE(string_view(reinterpret_cast<const char*>(output_region.data()), output_region.size()) == "abcdef");

    storage.consume(10);
    output_region = storage.data();
    REQUIRE(output_region.size() == 0);
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