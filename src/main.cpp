#include <nlohmann/json.hpp>

#include "media_session.hpp"
#include "settings.hpp"
#include "terminal_ui.hpp"
#include "websocket_server.hpp"
#include "websocket_session.hpp"

using namespace MonitorMedia;
using json = nlohmann::json;

std::string WStringToString(const std::wstring& wstr) {
  if (wstr.empty()) {
    return "";
  }

  int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    return "";
  }

  std::string result(size - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
  return result;
}

json ConvertToJson(std::string event, TrackInfo track_info) {
  return {{"event_name", event},
          {"payload",
           {{"title", track_info.title != L"" ? WStringToString(track_info.title) : ""},
            {"artist", track_info.artist != L"" ? WStringToString(track_info.artist) : ""},
            {"album", track_info.album != L"" ? WStringToString(track_info.album) : ""},
            {"app_name", track_info.app_name != L"" ? WStringToString(track_info.app_name) : ""},
            {"app_id", track_info.app_id != L"" ? WStringToString(track_info.app_id) : ""},
            {"position", track_info.current_position.count()},
            {"duration", track_info.duration.count()},
            {"is_playing", track_info.is_playing},
            {"has_media", track_info.has_media},
            {"thumbnail", track_info.thumbnail != L"" ? WStringToString(track_info.thumbnail) : ""}}}};
}

void show_help() {
  std::cout << "Media Monitor - A tool to monitor and broadcast media playback information\n\n"
            << "Usage: media_monitor_cli [OPTIONS]\n\n"
            << "Options:\n"
            << "  -h, --help        Show this help message and exit\n"
            << "  -l, --show-logs   Show WebSocket messages in the terminal\n";
  exit(0);
}

int main(int argc, char* argv[]) {
  bool show_logs = false;
  
  // Process command line arguments
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      show_help();
    } else if (strcmp(argv[i], "--show-logs") == 0 || strcmp(argv[i], "-l") == 0) {
      show_logs = true;
    }
  }

  try {
    init_apartment();
  } catch (const std::exception& e) {
    std::cerr << "Failed to initialize COM: " << e.what() << "\n";
    return 1;
  }

  try {
    auto& settings_ = SettingsManager::getSettings();
    auto ui_ = std::make_unique<TerminalUI>(70, 12);

    ui_->Initialize();
    ui_->DrawInitialUI();

    ui_->UpdateHeader(L"localhost", settings_.wsPort, 0);
    ui_->UpdateStatus(L"Stopped", L"");

    auto session_manager_ = std::make_unique<MediaSession>();
    auto server_ = std::make_unique<WebSocketServer>(settings_.wsPort);
    TrackIdentity previous_track_;
    SessionIdentity previous_session_;

    server_->on_connect([&server_, &ui_, &session_manager_, &settings_](WebSocketSession* connection) {
      connection->send(ConvertToJson("session_init", session_manager_->get_track_info()).dump());
      ui_->UpdateHeader(L"localhost", settings_.wsPort, server_->get_sessions_count());
    });

    server_->on_disconnect([&server_, &ui_, &settings_]() {
      ui_->UpdateHeader(L"localhost", settings_.wsPort, server_->get_sessions_count());
    });

    session_manager_->on_session_changed([&ui_, &server_, &session_manager_, &previous_session_, &show_logs](auto& track_info) {
      SessionIdentity current{track_info.app_id, track_info.is_playing};

      if (current == previous_session_) {
        return;
      }

      ui_->UpdateStatus(track_info.is_playing ? L"Playing" : L"Paused", track_info.app_name);
      ui_->UpdateTrackInfo(track_info.title, track_info.artist, track_info.album);
      ui_->UpdateProgress(track_info.current_position.count(), track_info.duration.count(), track_info.is_playing);
      auto json = ConvertToJson("session_changed", session_manager_->get_track_info()).dump();
      if (show_logs) {
        ui_->ShowLog(json);
      }
      server_->broadcast(json);

      previous_session_ = current;
    });

    session_manager_->on_track_changed([&ui_, &server_, &session_manager_, &previous_track_, &show_logs](auto& track_info) {
      TrackIdentity current{track_info.title, track_info.artist, track_info.album, track_info.is_playing};

      if (current == previous_track_) {
        return;
      }

      ui_->UpdateStatus(track_info.is_playing ? L"Playing" : L"Paused", track_info.app_name);
      ui_->UpdateTrackInfo(track_info.title, track_info.artist, track_info.album);

      if (!track_info.is_playing) {
        ui_->UpdateProgress(track_info.current_position.count(), track_info.duration.count(), track_info.is_playing);
      }

      auto json = ConvertToJson("track_changed", session_manager_->get_track_info()).dump();
      if (show_logs) {
        ui_->ShowLog(json);
      }
      server_->broadcast(json);

      previous_track_ = current;
    });

    session_manager_->on_position_changed([&ui_, &server_, &session_manager_, &previous_track_, &show_logs](auto& track_info) {
      if (track_info.is_playing) {
        ui_->UpdateProgress(track_info.current_position.count(), track_info.duration.count(), track_info.is_playing);
        auto json = ConvertToJson("position_changed", session_manager_->get_track_info()).dump();
        if (show_logs) {
          ui_->ShowLog(json);
        }
        server_->broadcast(json);
      }
    });

    server_->start();
    session_manager_->start();

    std::cin.get();

    ui_->Clear();
    ui_ = nullptr;
    server_->stop();
    server_ = nullptr;
    session_manager_ = nullptr;
  } catch (const hresult_error& e) {
    std::wcerr << L"WinRT Error: " << e.message().c_str() << L"\n";
    std::wcerr << L"HRESULT: 0x" << std::hex << e.code() << L"\n";
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}