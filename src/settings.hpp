#pragma once

#include <windows.h>
#include <string>

namespace MonitorMedia {
  struct Settings {
    bool wsEnabled = false;
    unsigned short wsPort = 7777;
    std::wstring appFilter = L"";
    std::wstring currentAppId = L"";
    bool eventMediaChange = true;
    bool eventSessionChange = true;
    bool eventDuration = true;
    bool eventPlayback = true;
    bool eventTimeline = false;
    bool showTrayNotification = true;
    bool minimizeToTray = true;

    int windowPosX = -1;
    int windowPosY = -1;
    std::wstring windowDevice = L"";
  };

  class SettingsManager {
  public:
    static SettingsManager& getInstance() {
      static SettingsManager instance;
      return instance;
    }

    const static Settings& getSettings();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

  private:
    SettingsManager();

    void SaveSettings(const Settings& new_settings);

  private:
    Settings settings_;

    std::wstring GetSettingsPath();

    void LoadSettings();
  };
} // namespace MonitorMedia
