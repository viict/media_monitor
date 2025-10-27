#pragma once

#include "media_session.hpp"
#include "track_info.hpp"

namespace MonitorMedia {
  MediaSession::MediaSession() :
      session_manager_(GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get()) {
    token_session_ = session_manager_.SessionsChanged([this](auto&&, auto&&) { session_changed(); });
  }

  MediaSession::~MediaSession() {
    if (current_session_) {
      unsubscribe_session(current_session_);
    }

    session_manager_.SessionsChanged(token_session_);
    running_ = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    progress_thread_.join();
  }

  void MediaSession::session_changed() {
    auto session = session_manager_.GetCurrentSession();
    subscribe_session(session);
    set_current_appId();
    set_current_appName();
    update_track_info(session);

    if (on_session_changed_) {
      on_session_changed_(current_track_);
    }
  }

  void MediaSession::properties_changed(GlobalSystemMediaTransportControlsSession session) {
    update_track_info(session);
  }

  void MediaSession::playback_info_changed(GlobalSystemMediaTransportControlsSession session) {
    update_track_info(session);
  }

  void MediaSession::timeline_properties_changed(GlobalSystemMediaTransportControlsSession session) {
    update_track_info(session);
  }

  void MediaSession::unsubscribe_session(GlobalSystemMediaTransportControlsSession session) {
    try {
      session.MediaPropertiesChanged(token_media_);
      session.PlaybackInfoChanged(token_playback_);
      session.TimelinePropertiesChanged(token_timeline_);
    } catch (...) {
    }
  }

  void MediaSession::subscribe_session(GlobalSystemMediaTransportControlsSession session) {
    if (!session) {
      return;
    }

    if (current_session_) {
      unsubscribe_session(current_session_);
    }

    current_session_ = session;
    token_media_ = session.MediaPropertiesChanged([this](auto&& session, auto&&) { properties_changed(session); });
    token_playback_ = session.PlaybackInfoChanged([this](auto&& session, auto&&) { playback_info_changed(session); });
    token_timeline_ =
        session.TimelinePropertiesChanged([this](auto&& session, auto&&) { timeline_properties_changed(session); });
  }

  void MediaSession::set_current_appId() {
    if (!current_session_)
      current_track_.app_id = L"Unknown";
    try {
      current_track_.app_id = current_session_.SourceAppUserModelId().c_str();
    } catch (...) {
      current_track_.app_id = L"Unknown";
    }
  }

  void MediaSession::set_current_appName() {
    if (!current_session_) {
      current_track_.app_name = L"Unknown";
      return;
    }

    try {
      auto id = get_current_appId();
      auto lowerId = id;
      std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), [](wchar_t c) { return std::towlower(c); });

      if (lowerId.find(L"spotify") != std::wstring::npos) {
        current_track_.app_name = L"Spotify";
      } else if (lowerId.find(L"msedge") != std::wstring::npos) {
        current_track_.app_name = L"Microsoft Edge";
      } else if (lowerId.find(L"chrome") != std::wstring::npos) {
        current_track_.app_name = L"Google Chrome";
      } else if (lowerId.find(L"firefox") != std::wstring::npos) {
        current_track_.app_name = L"Firefox";
      } else if (lowerId.find(L"Microsoft.ZuneMusic") != std::wstring::npos) {
        current_track_.app_name = L"Groove Music";
      } else if (lowerId.find(L"VLC") != std::wstring::npos) {
        current_track_.app_name = L"VLC";
      } else {
        current_track_.app_name = id;
      }
    } catch (...) {
      current_track_.app_name = L"Unknown";
    }
  }

  void MediaSession::update_track_info(GlobalSystemMediaTransportControlsSession session) {
    if (!session) {
      std::lock_guard<std::mutex> lock(track_mutex_);
      current_track_.has_media = false;
      current_track_.is_playing = false;

      if (on_track_changed_) {
        on_track_changed_(current_track_);
      }

      return;
    }

    try {
      auto props = session.TryGetMediaPropertiesAsync().get();
      current_track_.title = props.Title().c_str();
      current_track_.subtitle = props.Subtitle().c_str();
      current_track_.artist = props.Artist().c_str();
      current_track_.album = props.AlbumTitle().c_str();

      try {
        auto thumbRef = props.Thumbnail();
        if (thumbRef) {
          try {
            auto stream = thumbRef.OpenReadAsync().get();
            auto size = static_cast<uint32_t>(stream.Size());
            if (size > 0) {
              auto buffer = winrt::Windows::Storage::Streams::Buffer(size);
              auto readBuffer =
                  stream.ReadAsync(buffer, size, winrt::Windows::Storage::Streams::InputStreamOptions::None).get();

              auto base64 =
                  winrt::Windows::Security::Cryptography::CryptographicBuffer::EncodeToBase64String(readBuffer);

              // Build a data URI including the content type so it can be used in an <img src="">
              std::wstring dataUri;
              try {
                // Default to a common image type; try to read actual content type from the stream
                std::wstring contentType = L"image/jpeg";
                try {
                  auto ct = stream.ContentType();
                  if (!ct.empty()) {
                    contentType = ct.c_str();
                  }
                } catch (...) {
                  // keep default
                }

                dataUri = std::wstring(L"data:") + contentType + L";base64," + base64.c_str();
              } catch (...) {
                dataUri.clear();
              }

              current_track_.thumbnail = dataUri;
            } else {
              current_track_.thumbnail = L"";
            }
          } catch (...) {
            current_track_.thumbnail = L"";
          }
        } else {
          current_track_.thumbnail = L"";
        }
      } catch (...) {
        current_track_.thumbnail = L"";
      }

      auto playback = session.GetPlaybackInfo();
      auto playback_status = playback.PlaybackStatus();
      current_track_.is_playing = (playback_status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);

      auto timeline = session.GetTimelineProperties();
      auto raw_duration = timeline.EndTime();
      auto last_updated = timeline.LastUpdatedTime();
      auto position = timeline.Position();

      if (current_track_.is_playing) {
        // Get current time and calculate elapsed since last update
        auto now = winrt::clock::now();
        auto elapsed = now - last_updated;
        position = position + elapsed;

        // Clamp to duration
        if (position > raw_duration) {
          position = raw_duration;
        }
      }

      current_track_.position = std::chrono::duration_cast<std::chrono::seconds>(position);
      current_track_.duration = std::chrono::duration_cast<std::chrono::seconds>(raw_duration);
      current_track_.current_position = current_track_.position;

      // Important: Reset update time!
      current_track_.last_update_time = std::chrono::system_clock::now();
      current_track_.has_media = true;
    } catch (...) {
      current_track_.has_media = false;
    }

    if (on_track_changed_) {
      on_track_changed_(current_track_);
    }
  }

  void MediaSession::start() {
    // TODO: If this is filtered we need to check if filtered app is available
    session_changed();

    running_ = true;
    progress_thread_ = std::thread([&]() {
      while (running_) {
        TrackInfo track_copy;

        {
          std::lock_guard<std::mutex> lock(track_mutex_);

          if (current_track_.is_playing && current_track_.duration.count() > 0) {
            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - current_track_.last_update_time);
            auto current_position = current_track_.position + elapsed;

            if (current_position > current_track_.duration) {
              current_position = current_track_.duration;
            }

            current_track_.current_position = current_position;
          }

          track_copy = current_track_;
        }

        if (on_position_changed_) {
          on_position_changed_(track_copy);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
    });
  }

  const std::wstring& MediaSession::get_current_appId() const {
    return current_track_.app_id;
  }

  const std::wstring& MediaSession::get_current_appName() const {
    return current_track_.app_name;
  }

  const TrackInfo& MediaSession::get_track_info() const {
    return current_track_;
  }

  void MediaSession::on_session_changed(OnTrackChangedCallback callback) {
    on_session_changed_ = std::move(callback);
  }

  void MediaSession::on_track_changed(OnTrackChangedCallback callback) {
    on_track_changed_ = std::move(callback);
  }

  void MediaSession::on_position_changed(OnTrackChangedCallback callback) {
    on_position_changed_ = std::move(callback);
  }
} // namespace MonitorMedia
