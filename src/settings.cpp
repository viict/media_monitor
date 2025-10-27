#pragma once

#include "settings.hpp"

namespace MonitorMedia {
  const Settings& SettingsManager::getSettings() {
    static SettingsManager& instance = getInstance();
    return instance.settings_;
  }

  SettingsManager::SettingsManager() {
    LoadSettings();
  }

  void SettingsManager::SaveSettings(const Settings& new_settings) {
    std::wstring iniPath = GetSettingsPath();

    WritePrivateProfileString(L"WebSocket", L"Enabled", new_settings.wsEnabled ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"WebSocket", L"Port", std::to_wstring(new_settings.wsPort).c_str(), iniPath.c_str());
    WritePrivateProfileString(L"Filter", L"AppFilter", new_settings.appFilter.c_str(), iniPath.c_str());
    WritePrivateProfileString(L"Events", L"MediaChange", new_settings.eventMediaChange ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(
        L"Events", L"SessionChange", new_settings.eventSessionChange ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"Events", L"Duration", new_settings.eventDuration ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"Events", L"Playback", new_settings.eventPlayback ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"Events", L"Timeline", new_settings.eventTimeline ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(
        L"Tray", L"ShowNotification", new_settings.showTrayNotification ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"Tray", L"MinimizeToTray", new_settings.minimizeToTray ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileString(L"Window", L"posX", std::to_wstring(new_settings.windowPosX).c_str(), iniPath.c_str());
    WritePrivateProfileString(L"Window", L"posY", std::to_wstring(new_settings.windowPosY).c_str(), iniPath.c_str());
    WritePrivateProfileString(L"Window", L"Device", new_settings.windowDevice.c_str(), iniPath.c_str());
  }

  std::wstring SettingsManager::GetSettingsPath() {
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    return exePath.substr(0, pos + 1) + L"settings.ini";
  }

  void SettingsManager::LoadSettings() {
    std::wstring iniPath = GetSettingsPath();
    wchar_t buffer[256];

    GetPrivateProfileString(L"WebSocket", L"Enabled", L"0", buffer, 256, iniPath.c_str());
    settings_.wsEnabled = (buffer[0] == L'1');

    GetPrivateProfileString(L"WebSocket", L"Port", L"7777", buffer, 256, iniPath.c_str());
    settings_.wsPort = static_cast<unsigned short>(std::stoul(buffer));

    GetPrivateProfileString(L"Filter", L"AppFilter", L"", buffer, 256, iniPath.c_str());
    settings_.appFilter = buffer;

    GetPrivateProfileString(L"Events", L"MediaChange", L"1", buffer, 256, iniPath.c_str());
    settings_.eventMediaChange = (buffer[0] == L'1');
    GetPrivateProfileString(L"Events", L"SessionChange", L"1", buffer, 256, iniPath.c_str());
    settings_.eventSessionChange = (buffer[0] == L'1');
    GetPrivateProfileString(L"Events", L"Duration", L"1", buffer, 256, iniPath.c_str());
    settings_.eventDuration = (buffer[0] == L'1');
    GetPrivateProfileString(L"Events", L"Playback", L"1", buffer, 256, iniPath.c_str());
    settings_.eventPlayback = (buffer[0] == L'1');
    GetPrivateProfileString(L"Events", L"Timeline", L"0", buffer, 256, iniPath.c_str());
    settings_.eventTimeline = (buffer[0] == L'1');

    GetPrivateProfileString(L"Tray", L"ShowNotification", L"1", buffer, 256, iniPath.c_str());
    settings_.showTrayNotification = (buffer[0] == L'1');
    GetPrivateProfileString(L"Tray", L"MinimizeToTray", L"1", buffer, 256, iniPath.c_str());
    settings_.minimizeToTray = (buffer[0] == L'1');

    GetPrivateProfileString(L"Window", L"posX", L"-1", buffer, 256, iniPath.c_str());
    settings_.windowPosX = std::stoi(buffer);
    GetPrivateProfileString(L"Window", L"posY", L"-1", buffer, 256, iniPath.c_str());
    settings_.windowPosY = std::stoi(buffer);
    GetPrivateProfileString(L"Window", L"Device", L"", buffer, 256, iniPath.c_str());
    settings_.windowDevice = buffer;
  }
} // namespace MonitorMedia
