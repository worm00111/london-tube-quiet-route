#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

using NetworkMonitor::WebSocketClient;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
    // Connection targets
    const std::string url {"echo.websocket.org"};
    const std::string port {"80"};
    const std::string message {"Hello WebSocket"};
    std::string echo{};

    // Always start with an I/O context object.
    boost::asio::io_context ioc {};

    // The class under test
    WebSocketClient client {url, port, ioc};

    // We use these flags to check that the connection, send, receive functions
    // work as expected.
    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool messageMatches {false};
    bool disconnected {false};

    // Our own callbacks
    auto onSend {[&messageSent](auto ec) {
        messageSent = !ec;
    }};
    auto onConnect {[&client, &connected, &onSend, &message](auto ec) {
        connected = !ec;
        if (!ec) {
            client.Send(message, onSend);
        }
    }};
    auto onClose {[&disconnected](auto ec) {
        disconnected = !ec;
    }};
    auto onReceive {[&client,
                      &onClose,
                      &messageReceived,
                      &messageMatches,
                      &message,
                      &echo](auto ec, auto received) {
        messageReceived = !ec;
        messageMatches = message == received;
        echo = std::move(received);
        client.Close(onClose);
    }};

    // We must call io_context::run for asynchronous callbacks to run.
    client.Connect(onConnect, onReceive);
    ioc.run();

    // When we get here, the io_context::run function has run out of work to do.
    BOOST_CHECK(connected);
    BOOST_CHECK(messageSent);
    BOOST_CHECK(messageReceived);
    BOOST_CHECK(disconnected);
    BOOST_CHECK_EQUAL(message, echo);
}

BOOST_AUTO_TEST_SUITE_END();