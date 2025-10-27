#pragma once

#include <chrono>
#include <cwctype>
#include <functional>
#include <mutex>
#include <thread>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <winrt/windows.security.cryptography.h>
#include "track_info.hpp"

using namespace winrt;
using namespace Windows::Media::Control;

namespace MonitorMedia {
  using OnTrackChangedCallback = std::function<void(TrackInfo&)>;

  class MediaSession {
  private:
    GlobalSystemMediaTransportControlsSessionManager session_manager_{nullptr};
    GlobalSystemMediaTransportControlsSession current_session_{nullptr};

    TrackInfo current_track_;
    std::mutex track_mutex_;

    event_token token_media_;
    event_token token_playback_;
    event_token token_timeline_;
    event_token token_session_;

    std::thread progress_thread_;
    bool running_ = false;

    OnTrackChangedCallback on_session_changed_;
    OnTrackChangedCallback on_track_changed_;
    OnTrackChangedCallback on_position_changed_;

    void session_changed();

    void properties_changed(GlobalSystemMediaTransportControlsSession session);

    void playback_info_changed(GlobalSystemMediaTransportControlsSession session);

    void timeline_properties_changed(GlobalSystemMediaTransportControlsSession session);

    void unsubscribe_session(GlobalSystemMediaTransportControlsSession session);

    void subscribe_session(GlobalSystemMediaTransportControlsSession session);

    void set_current_appId();

    void set_current_appName();

    void update_track_info(GlobalSystemMediaTransportControlsSession session);

  public:
    MediaSession();

    ~MediaSession();

    void start();

    const std::wstring& get_current_appId() const;

    const std::wstring& get_current_appName() const;

    const TrackInfo& get_track_info() const;

    void on_session_changed(OnTrackChangedCallback callback);

    void on_track_changed(OnTrackChangedCallback callback);

    void on_position_changed(OnTrackChangedCallback callback);
  };
} // namespace MonitorMedia
