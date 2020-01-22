cmake_minimum_required(VERSION 3.15)

include(cmake/HunterGate.cmake)

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.240.tar.gz"
        SHA1 "ca19f3769e6c80cfdd19d8b12ba5102c27b074e0"
)

project(DynamicBuffer)


hunter_add_package(Boost COMPONENTS system)
hunter_add_package(Catch)
hunter_add_package(OpenSSL)

find_package(Boost CONFIG COMPONENTS system)
find_package(Catch2)
find_package(OpenSSL)
find_package(Threads)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")
file(GLOB_RECURSE include_files CONFIGURE_DEPENDS "include/*.hpp" "include/*.ipp")
add_executable(check ${src_files})
target_include_directories(check PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(check PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(check PRIVATE Boost::system Catch2::Catch2 OpenSSL::Crypto OpenSSL::SSL Threads::Threads)

target_compile_definitions(check PRIVATE NO_FLAT_STORAGE=1)
target_compile_definitions(check PRIVATE NO_MULTI_STORAGE=1)
target_compile_definitions(check PRIVATE NO_CIRCULAR_STORAGE=1)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(check PRIVATE -Werror -Wall -Wextra -pedantic)
endif()

