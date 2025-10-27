#pragma once

#include <windows.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace MonitorMedia {
  class TerminalUI {
  private:
    int width_;
    int height_;

    static constexpr int HEADER_ROW = 2;
    static constexpr int STATUS_ROW = 4;
    static constexpr int TITLE_ROW = 6;
    static constexpr int ARTIST_ROW = 7;
    static constexpr int ALBUM_ROW = 8;
    static constexpr int SEPARATOR_ROW = 9;
    static constexpr int PROGRESS_ROW = 10;

    HANDLE output_handle_;

    enum Color { BLACK = 0, RED = 1, GREEN = 2, YELLOW = 3, BLUE = 4, MAGENTA = 5, CYAN = 6, WHITE = 7 };

    void WriteUTF8String(const std::string& utf8str);
    void WriteWString(const std::wstring& wstr);

    void MoveCursor(int row, int col);
    void MoveCursor(size_t row, size_t col);
    void MoveCursor(int row, size_t col);
    void SetColor(int fg, int bg, bool bright);
    void ResetColor();
    void ClearLine();
    void HideCursor();
    void ShowCursorInternal();
    void DrawHorizontalLine(int row, bool isTop, bool isBottom);
    void DrawEmptyLine(int row, const std::wstring& content);

    std::wstring FormatTime(long long seconds);
    std::wstring TruncateString(const std::wstring& str, size_t maxLen);

  public:
    TerminalUI(int width, int height);

    ~TerminalUI();

    void Initialize();
    void DrawInitialUI();

    void UpdateHeader(const std::wstring& wsHost, unsigned short wsPort, size_t clientCount);
    void UpdateStatus(const std::wstring& status, const std::wstring& app);
    void UpdateTrackInfo(const std::wstring& title, const std::wstring& artist, const std::wstring& album);
    void UpdateProgress(long long currentSec, long long totalSec, bool isPlaying);

    void ShowCursor(bool show);
    void ShowLog(const std::string& json);
    void Clear();
  };

} // namespace MonitorMedia