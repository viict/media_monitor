#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include "websocket_session.hpp"

namespace MonitorMedia {
  using OnConnectCallback = std::function<void(WebSocketSession*)>;
  using OnDisconnectCallback = std::function<void()>;

  class WebSocketServer {
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    std::set<std::shared_ptr<WebSocketSession>> sessions_;
    std::mutex sessionsMutex_;
    std::thread ioThread_;
    bool running_ = false;

    OnConnectCallback on_connect_;
    OnDisconnectCallback on_disconnect_;

  public:
    WebSocketServer(unsigned short port);
    ~WebSocketServer();

    void start();
    void stop();
    void broadcast(const std::string& message);

    size_t get_sessions_count();

    void on_connect(OnConnectCallback callback);
    void on_disconnect(OnDisconnectCallback callback);

  private:
    void do_accept();
    void remove_session(std::shared_ptr<WebSocketSession> session);
  };
} // namespace MonitorMedia