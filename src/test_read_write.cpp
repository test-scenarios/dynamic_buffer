#include "config.hpp"
#include <catch2/catch.hpp>
#include <boost/beast/_experimental/test/stream.hpp>
#include <boost/beast/_experimental/test/fail_count.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/is_beast_v2_dynamic_buffer.hpp>
#include <boost/beast/circular_storage.hpp>
#include <boost/beast/flat_storage.hpp>
#include <boost/beast/multi_storage.hpp>
#include <boost/beast/static_storage.hpp>

namespace project_test {

using namespace boost::beast;

template<class Stream,
    class BeastV2DynamicBuffer,
    class Handler>
struct read_until_crlf_op
    : boost::asio::coroutine
      , async_base<Handler, boost::beast::executor_type<Stream>>
{
    static constexpr std::size_t
    chunk_size()
    { return 4096; }

    read_until_crlf_op(
        Stream &stream,
        BeastV2DynamicBuffer dyn_buf,
        Handler handler)
        : async_base<Handler,
        boost::beast::executor_type<Stream>>(
        std::move(handler),
        stream.get_executor())
        , stream_(stream)
        , dyn_buf_(dyn_buf)
    {
        (*this)(error_code(), 0);
    }

#include <boost/asio/yield.hpp>

    void
    operator()(
        boost::beast::error_code ec,
        std::size_t bytes_transferred)
    {
        reenter(this)
        for (;;)
        {
            end_of_sequence_ = find_crlf();
            if (end_of_sequence_)
                goto completion;

            to_read_ = std::min(chunk_size(), dyn_buf_.max_size() - dyn_buf_.size());

            if (to_read_ == 0)
            {
                ec = http::error::body_limit;
                goto completion;
            }

            is_continuation_ = true;

            // note: this is safe. dynamic_buffers are lightweight references to storage which do not
            // lose state when moved from

            yield
            {
                auto buffers = dyn_buf_.prepare(to_read_);
                auto &stream = stream_;
                stream.async_read_some(buffers, std::move(*this));
            }

            dyn_buf_.dispose_input(to_read_ - bytes_transferred);

            if (ec)
                goto completion;
        }

        return;

        completion:

        this->complete(is_continuation_, ec, end_of_sequence_);
    }

#include <boost/asio/unyield.hpp>

private:
    std::size_t
    find_crlf() const
    {
        auto s = boost::beast::buffers_to_string(dyn_buf_.data());
        auto pos = s.find("\r\n");
        auto result = (pos == std::string::npos)
                      ? std::size_t(0)
                      : std::size_t(pos + 2);
        return result;
    }

    Stream &stream_;
    BeastV2DynamicBuffer dyn_buf_;
    std::size_t to_read_;
    std::size_t end_of_sequence_ = 0;
    bool is_continuation_ = false;
};

struct run_read_until_crlf_op
{
    template<
        class ReadHandler,
        class AsyncReadStream,
        class DynamicBuffer>
    void
    operator()(
        ReadHandler &&h,
        AsyncReadStream *s,
        DynamicBuffer b)
    {
        using namespace boost::beast;

        // If you get an error on the following line it means
        // that your handler does not meet the documented type
        // requirements for the handler.

        static_assert(
            detail::is_invocable<ReadHandler,
                void(
                    error_code,
                    std::size_t)>::value,
            "ReadHandler type requirements not met");

        read_until_crlf_op<
            AsyncReadStream,
            DynamicBuffer,
            typename std::decay<ReadHandler>::type>(*s, b,
                                                    std::forward<ReadHandler>(h));
    }

};

template<
    class AsyncReadStream,
    class DynamicBuffer,
    class ReadHandler>
BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
async_read_until_crlf(
    AsyncReadStream &stream,
    DynamicBuffer buffer,
    ReadHandler &&handler)
{
    static_assert(is_async_read_stream<AsyncReadStream>::value,
                  "AsyncReadStream type requirements not met");
    static_assert(
        is_beast_v2_dynamic_buffer<DynamicBuffer>::value,
        "DynamicBuffer type requirements not met");
    return net::async_initiate<
        ReadHandler,
        void(
            error_code,
            std::size_t)>(
        run_read_until_crlf_op(),
        handler,
        &stream,
        buffer);
}


struct make_static
{
    enum
        : std::size_t
    {
        max_capacity = 64
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
        max_capacity = 64
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
        max_capacity = 64
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
        max_capacity = 64
    };

    auto
    operator()() const -> boost::beast::multi_storage
    {
        return boost::beast::multi_storage(max_capacity);
    }
};

}


TEMPLATE_TEST_CASE("read_write", "", project_test::make_flat, project_test::make_static, project_test::make_circular,
                   project_test::make_multi)
{
    using namespace boost::beast;

    net::io_context ioc(1);

    auto test_run = [&] {
        ioc.run();
        if (ioc.stopped())
            ioc.restart();
    };

    auto client_stream = test::stream(ioc);
    auto server_stream = test::connect(client_stream);

    write(server_stream, net::buffer(std::string("1 the cat sat on the mat\r\n"
                                                 "2 the cat sat on the mat\r\n"
                                                 "3 the cat sat on the mat\r\n"
                                                 "4 the cat sat on the mat\r\n"
                                                 "5 the cat sat on the mat\r\n"
                                                 "6 the cat sat on the mat\r\n"
                                                 "7 the cat sat on the mat\r\n"
                                                 "8 the cat sat on the mat\r\n"
                                                 "9 the cat gave up")));
    server_stream.close();

    auto storage = TestType()();
    auto dyn_buf = dynamic_buffer(storage);

    auto expected_error = error_code();
    auto expected_data = std::string("1 the cat sat on the mat\r\n");
    auto handler = [&](
        error_code const &ec,
        std::size_t bytes_tramsferred) {
        CHECK(ec.value() == expected_error.value());
        CHECK(ec.category().name() == expected_error.category().name());
        CHECK(ec.message() == expected_error.message());

        if (!ec)
        {
            CHECK(bytes_tramsferred == expected_data.size());
            CHECK(buffers_to_string(dyn_buf.data()).substr(0, bytes_tramsferred) == expected_data);
        }
        dyn_buf.consume(bytes_tramsferred);
    };

    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();

    expected_data = std::string("2 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("3 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("4 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("5 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("6 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("7 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();
    expected_data = std::string("8 the cat sat on the mat\r\n");
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();

    expected_data = std::string("9 the cat gave up");
    expected_error = net::error::eof;
    project_test::async_read_until_crlf(client_stream, dyn_buf, handler);
    test_run();


}
