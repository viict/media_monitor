#pragma once

#include "websocket_server.hpp"
#include "websocket_session.hpp"

namespace MonitorMedia {
  WebSocketServer::WebSocketServer(unsigned short port) : acceptor_(ioc_) {
    try {
      beast::error_code ec;
      tcp::endpoint endpoint{tcp::v4(), port};

      acceptor_.open(endpoint.protocol(), ec);
      if (ec) {
        throw std::runtime_error("Failed to open acceptor: " + ec.message());
      }

      acceptor_.set_option(net::socket_base::reuse_address(true), ec);
      if (ec) {
        throw std::runtime_error("Failed to set reuse_address: " + ec.message());
      }

      acceptor_.bind(endpoint, ec);
      if (ec) {
        throw std::runtime_error("Failed to bind: " + ec.message());
      }

      acceptor_.listen(net::socket_base::max_listen_connections, ec);
      if (ec) {
        throw std::runtime_error("Failed to listen: " + ec.message());
      }
    } catch (const std::exception& e) {
      std::cerr << "Server initialization error: " << e.what() << "\n";
      throw;
    }
  }

  WebSocketServer::~WebSocketServer() {
    stop();
  }

  void WebSocketServer::start() {
    running_ = true;
    do_accept();
    ioThread_ = std::thread([this]() {
      try {
        ioc_.run();
      } catch (const std::exception& e) {
        std::cerr << "IO thread exception: " << e.what() << "\n";
      }
    });
  }

  void WebSocketServer::stop() {
    if (!running_) {
      return;
    }

    running_ = false;

    beast::error_code ec;
    acceptor_.close(ec);

    for (auto session : sessions_) {
      session->close();
    }

    ioc_.stop();

    if (ioThread_.joinable()) {
      ioThread_.join();
    }
  }

  void WebSocketServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(sessionsMutex_);

    auto it = sessions_.begin();
    while (it != sessions_.end()) {
      if ((*it)->is_open()) {
        (*it)->send(message);
        ++it;
      } else {
        it = sessions_.erase(it);
      }
    }
  }

  size_t WebSocketServer::get_sessions_count() {
    return sessions_.size();
  }

  void WebSocketServer::on_connect(OnConnectCallback callback) {
    on_connect_ = std::move(callback);
  }

  void WebSocketServer::on_disconnect(OnDisconnectCallback callback) {
    on_disconnect_ = std::move(callback);
  }

  void WebSocketServer::do_accept() {
    if (!running_)
      return;

    acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) {
      if (!ec) {
        auto session = std::make_shared<WebSocketSession>(
            std::move(socket),
            [this](auto session) { remove_session(session); },
            [this](auto session) {
              {
                std::lock_guard<std::mutex> lock(sessionsMutex_);
                sessions_.insert(session);
              }

              if (on_connect_) {
                on_connect_(session.get());
              }
            });

        session->run();
      }

      if (running_) {
        do_accept();
      }
    });
  }

  void WebSocketServer::remove_session(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessions_.erase(session);

    if (on_disconnect_) {
      on_disconnect_();
    }
  }
} // namespace MonitorMedia