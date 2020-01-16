#include <iostream>
#include "explain.hpp"
#include "config.hpp"

namespace program {

    namespace websocket = beast::websocket;
    using tcp = net::ip::tcp;

    using namespace std::literals;

    int
    run()
    {
//        std::string data;
//        auto sv1 = string_buffer_v1(data);
//        auto sv2 = string_buffer_v2(data);

        std::cout << "string_buffer_v1 is dynamic_buffer_v1? " << std::boolalpha << net::is_dynamic_buffer_v1<string_buffer_v1>::value << '\n';
        std::cout << "string_buffer_v1 is dynamic_buffer_v2? " << std::boolalpha << net::is_dynamic_buffer_v2<string_buffer_v1>::value << '\n';

        std::cout << "string_buffer_v2 is dynamic_buffer_v1? " << std::boolalpha << net::is_dynamic_buffer_v1<string_buffer_v2>::value << '\n';
        std::cout << "string_buffer_v2 is dynamic_buffer_v2? " << std::boolalpha << net::is_dynamic_buffer_v2<string_buffer_v2>::value << '\n';

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