#pragma once

#define _WIN32_WINNT 0x0A00 // Windows 10
#ifndef WINVER
  #define WINVER 0x0A00
#endif
#include <atomic>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace MonitorMedia {
  class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::function<void(std::shared_ptr<WebSocketSession>)> onClose_;
    std::function<void(std::shared_ptr<WebSocketSession>)> onReady_;
    std::deque<std::shared_ptr<std::string>> write_queue_;
    std::mutex write_mutex_;
    std::atomic<bool> writing_{false};

  public:
    WebSocketSession(tcp::socket socket,
                     std::function<void(std::shared_ptr<WebSocketSession>)> onClose,
                     std::function<void(std::shared_ptr<WebSocketSession>)> onReady);

    void run();
    bool is_open() const;
    void close();
    void send(std::string message);

  private:
    void do_write();
    void do_read();

    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t);
  };
} // namespace MonitorMedia