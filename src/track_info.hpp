#pragma once

#include <chrono>
#include <iostream>

namespace MonitorMedia {

  struct TrackInfo {
    std::chrono::system_clock::time_point last_update_time;
    std::chrono::seconds current_position{0};
    std::chrono::seconds position{0};
    std::chrono::seconds duration{0};
    std::wstring title = L"No media playing";
    std::wstring subtitle = L"";
    std::wstring artist = L"";
    std::wstring album = L"";
    std::wstring app_name = L"";
    std::wstring app_id = L"";
    std::wstring thumbnail = L"";
    bool is_playing = false;
    bool has_media = false;
  };

  struct TrackIdentity {
    std::wstring title;
    std::wstring artist;
    std::wstring album;
    bool is_playing;

    bool operator==(const TrackIdentity& other) const {
      return title == other.title && artist == other.artist && album == other.album && is_playing == other.is_playing;
    }

    bool operator!=(const TrackIdentity& other) const {
      return !(*this == other);
    }
  };

  struct SessionIdentity {
    std::wstring app_id;
    bool is_playing;

    bool operator==(const SessionIdentity& other) const {
      return app_id == other.app_id && is_playing == other.is_playing;
    }

    bool operator!=(const SessionIdentity& other) const {
      return !(*this == other);
    }
  };

} // namespace MonitorMedia
