#define BOOST_TEST_MODULE notifier
#include <boost/test/unit_test.hpp>

#include <yandex/contest/invoker/notifier/BlockStream.hpp>

#include <boost/asio.hpp>

namespace ya = yandex::contest::invoker;
namespace yan = ya::notifier;

BOOST_AUTO_TEST_SUITE(notifier)

struct SerializationFactory
{
    SerializationFactory():
        socket1(ioService),
        socket2(ioService)
    {
        boost::asio::local::connect_pair(socket1, socket2);
    }

    typedef boost::asio::local::stream_protocol::socket Socket;
    boost::asio::io_service ioService;
    Socket socket1, socket2;
};

BOOST_FIXTURE_TEST_SUITE(serialization, SerializationFactory)

BOOST_AUTO_TEST_CASE(BlockStream)
{
    std::string bs1Data, bs2Data;
    yan::BlockStream<Socket> bs1(socket1), bs2(socket2);
    bs1.async_write("first request",
        [&](const boost::system::error_code &ec)
        {
            BOOST_REQUIRE(!ec);
            bs1.async_read(bs1Data,
                [&](const boost::system::error_code &ec)
                {
                    BOOST_REQUIRE(!ec);
                    BOOST_CHECK_EQUAL(bs1Data, "first response");
                    bs1.async_write("second request",
                        [&](const boost::system::error_code &ec)
                        {
                            BOOST_REQUIRE(!ec);
                            bs1.async_read(bs1Data,
                                [&](const boost::system::error_code &ec)
                                {
                                    BOOST_REQUIRE(!ec);
                                    BOOST_CHECK_EQUAL(bs1Data, "second response");
                                    bs1.close();
                                });
                        });
                });
        });
    bs2.async_read(bs2Data,
        [&](const boost::system::error_code &ec)
        {
            BOOST_REQUIRE(!ec);
            BOOST_CHECK_EQUAL(bs2Data, "first request");
            bs2.async_write("first response",
                [&](const boost::system::error_code &ec)
                {
                    BOOST_REQUIRE(!ec);
                    bs2.async_read(bs2Data,
                        [&](const boost::system::error_code &ec)
                        {
                            BOOST_REQUIRE(!ec);
                            BOOST_CHECK_EQUAL(bs2Data, "second request");
                            bs2.async_write("second response",
                                [&](const boost::system::error_code &ec)
                                {
                                    BOOST_REQUIRE(!ec);
                                    bs2.async_read(bs2Data,
                                        [&](const boost::system::error_code &ec)
                                        {
                                            BOOST_REQUIRE_EQUAL(
                                                ec,
                                                boost::asio::error::eof
                                            );
                                        });
                                });
                        });
                });
        });
    ioService.run();
}

BOOST_AUTO_TEST_SUITE_END() // serialization

BOOST_AUTO_TEST_SUITE_END() // notifier
