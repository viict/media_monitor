#pragma once

#include "websocket_session.hpp"

namespace MonitorMedia {
  WebSocketSession::WebSocketSession(tcp::socket socket,
                                     std::function<void(std::shared_ptr<WebSocketSession>)> onClose,
                                     std::function<void(std::shared_ptr<WebSocketSession>)> onReady) :
      ws_(std::move(socket)), onClose_(onClose), onReady_(onReady) {}

  void WebSocketSession::run() {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    ws_.async_accept(beast::bind_front_handler(&WebSocketSession::on_accept, shared_from_this()));
  }

  void WebSocketSession::send(std::string message) {
    auto msg = std::make_shared<std::string>(std::move(message));

    net::post(ws_.get_executor(), [self = shared_from_this(), msg]() {
      {
        std::lock_guard<std::mutex> lock(self->write_mutex_);
        self->write_queue_.push_back(msg);
      }

      if (!self->writing_.exchange(true)) {
        self->do_write();
      }
    });
  }

  bool WebSocketSession::is_open() const {
    return ws_.is_open();
  }

  void WebSocketSession::close() {
    net::post(ws_.get_executor(), [self = shared_from_this()]() {
      beast::error_code ec;
      self->ws_.close(websocket::close_code::normal, ec);
    });
  }

  void WebSocketSession::do_read() {
    ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::on_read, shared_from_this()));
  }

  void WebSocketSession::do_write() {
    std::shared_ptr<std::string> msg;

    {
      std::lock_guard<std::mutex> lock(write_mutex_);
      if (write_queue_.empty()) {
        writing_ = false;
        return;
      }

      msg = write_queue_.front();
    }

    ws_.async_write(net::buffer(*msg), [self = shared_from_this(), msg](beast::error_code ec, std::size_t) {
      {
        std::lock_guard<std::mutex> lock(self->write_mutex_);
        self->write_queue_.pop_front();
      }

      if (ec) {
        self->writing_ = false;
        if (self->onClose_) {
          self->onClose_(self);
        }
        return;
      }

      self->do_write();
    });
  }

  void WebSocketSession::on_accept(beast::error_code ec) {
    if (ec) {
      return;
    }

    if (onReady_) {
      onReady_(shared_from_this());
    }

    do_read();
  }

  void WebSocketSession::on_read(beast::error_code ec, std::size_t) {
    if (ec) {
      if (onClose_) {
        onClose_(shared_from_this());
      }
      return;
    }

    buffer_.consume(buffer_.size());
    do_read();
  }
} // namespace MonitorMedia