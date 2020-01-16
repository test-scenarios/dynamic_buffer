#include "config.hpp"

#include "explain.hpp"
#include <iostream>

namespace program {

using string_buffer_v1 =
net::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;

using string_buffer_v2 =
net::v2::boost::asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;

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

    std::cout << "string_buffer_v2 is dynamic_buffer_v1? " << std::boolalpha
              << net::is_dynamic_buffer_v1<string_buffer_v2>::value << '\n';
    std::cout << "string_buffer_v2 is dynamic_buffer_v2? " << std::boolalpha
              << net::is_dynamic_buffer_v2<string_buffer_v2>::value << '\n';

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