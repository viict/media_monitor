#pragma once

#include "terminal_ui.hpp"

namespace MonitorMedia {

  TerminalUI::TerminalUI(int width = 70, int height = 12) : width_(width), height_(height) {}

  TerminalUI::~TerminalUI() {
    ShowCursorInternal();
    ResetColor();
    WriteWString(L"\n");
  }

  void TerminalUI::WriteUTF8String(const std::string& utf8str) {
    DWORD written;
    WriteConsoleA(output_handle_, utf8str.c_str(), static_cast<DWORD>(utf8str.length()), &written, NULL);
  }

  void TerminalUI::WriteWString(const std::wstring& wstr) {
    if (wstr.empty()) {
      return;
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8str[0], size, nullptr, nullptr);

    DWORD written;
    WriteConsoleA(output_handle_, utf8str.c_str(), static_cast<DWORD>(utf8str.length()), &written, NULL);
  }

  void TerminalUI::MoveCursor(int row, int col) {
    WriteWString(L"\033[" + std::to_wstring(row) + L";" + std::to_wstring(col) + L"H");
  }

  void TerminalUI::MoveCursor(size_t row, size_t col) {
    WriteWString(L"\033[" + std::to_wstring(row) + L";" + std::to_wstring(col) + L"H");
  }

  void TerminalUI::MoveCursor(int row, size_t col) {
    WriteWString(L"\033[" + std::to_wstring(row) + L";" + std::to_wstring(col) + L"H");
  }

  void TerminalUI::SetColor(int fg, int bg = -1, bool bright = false) {
    if (bright) {
      WriteWString(L"\033[9" + std::to_wstring(fg) + L"m");
    } else {
      WriteWString(L"\033[3" + std::to_wstring(fg) + L"m");
    }
    if (bg >= 0) {
      WriteWString(L"\033[4" + std::to_wstring(bg) + L"m");
    }
  }

  void TerminalUI::ResetColor() {
    WriteWString(L"\033[0m");
  }

  void TerminalUI::ClearLine() {
    WriteWString(L"\033[K");
  }

  void TerminalUI::HideCursor() {
    WriteWString(L"\033[?25l");
  }

  void TerminalUI::ShowCursorInternal() {
    WriteWString(L"\033[?25h");
  }

  void TerminalUI::DrawHorizontalLine(int row, bool isTop, bool isBottom) {
    MoveCursor(row, 1);

    if (isTop) {
      WriteWString(L"┌");
    } else if (isBottom) {
      WriteWString(L"└");
    } else {
      WriteWString(L"├");
    }

    for (int i = 0; i < width_ - 2; i++) {
      WriteWString(L"─");
    }

    if (isTop) {
      WriteWString(L"┐");
    } else if (isBottom) {
      WriteWString(L"┘");
    } else {
      WriteWString(L"┤");
    }
  }

  void TerminalUI::DrawEmptyLine(int row, const std::wstring& content = L"") {
    MoveCursor(row, 1);
    WriteWString(L"│ ");
    WriteWString(content);

    size_t contentLen = content.length();
    for (size_t i = contentLen; i < width_ - 4; i++) {
      WriteWString(L" ");
    }

    WriteWString(L" │");
  }

  std::wstring TerminalUI::FormatTime(long long seconds) {
    auto mins = seconds / 60;
    auto secs = seconds % 60;
    std::wostringstream oss;
    oss << std::setfill(L'0') << std::setw(2) << std::to_wstring(mins) << L":" << std::setfill(L'0') << std::setw(2)
        << std::to_wstring(secs);
    return oss.str();
  }

  std::wstring TerminalUI::TruncateString(const std::wstring& str, size_t maxLen) {
    if (str.length() <= maxLen)
      return str;
    return str.substr(0, maxLen - 3) + L"...";
  }

  void TerminalUI::Initialize() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::setlocale(LC_CTYPE, ".UTF8");

    output_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(output_handle_, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(output_handle_, dwMode);

    HideCursor();
  }

  void TerminalUI::DrawInitialUI() {
    Clear();

    // Top border
    DrawHorizontalLine(HEADER_ROW - 1, true, false);

    // Header content (will be updated later)
    DrawEmptyLine(HEADER_ROW, L"");

    // Separator after header
    DrawHorizontalLine(STATUS_ROW - 1, false, false);
    DrawEmptyLine(STATUS_ROW + 1, L"");

    // Status line
    DrawEmptyLine(STATUS_ROW, L"");

    // Track info
    DrawEmptyLine(TITLE_ROW, L"");
    DrawEmptyLine(ARTIST_ROW, L"");
    DrawEmptyLine(ALBUM_ROW, L"");

    // Separator before progress
    DrawHorizontalLine(SEPARATOR_ROW, false, false);

    // Progress bar
    DrawEmptyLine(PROGRESS_ROW, L"");

    // Bottom border
    DrawHorizontalLine(PROGRESS_ROW + 1, false, true);
  }

  void TerminalUI::UpdateHeader(const std::wstring& wsHost, unsigned short wsPort, size_t clientCount) {
    MoveCursor(HEADER_ROW, 3);

    SetColor(CYAN, -1, true);
    WriteWString(L"♪ Media Monitor");
    ResetColor();

    std::wostringstream right;
    right << L"WS: " << wsHost << L":" << wsPort << L"  ";

    SetColor(GREEN, -1, false);
    right << "👥 " << clientCount << L" client" << (clientCount != 1 ? L"s" : L"");

    std::wstring rightStr = L"WS: " + wsHost + L":" + std::to_wstring(wsPort) + L"   " + std::to_wstring(clientCount) +
                            L" client" + (clientCount != 1 ? L"s" : L"");
    size_t rightPos = width_ - rightStr.length() - 3;

    MoveCursor(HEADER_ROW, rightPos);
    WriteWString(L"WS: " + wsHost + L":" + std::to_wstring(wsPort) + L"  ");
    SetColor(GREEN, -1, false);
    WriteWString(L"👥 " + std::to_wstring(clientCount) + L" client" + (clientCount != 1 ? L"s" : L""));
    ResetColor();
  }

  void TerminalUI::UpdateStatus(const std::wstring& status, const std::wstring& app) {
    MoveCursor(STATUS_ROW, 3);
    ClearLine();

    if (status == L"Playing") {
      SetColor(GREEN, -1, true);
      WriteWString(L"🟢 Playing");
    } else if (status == L"Paused") {
      SetColor(YELLOW, -1, true);
      WriteWString(L"🟡 Paused");
    } else if (status == L"Stopped") {
      SetColor(RED, -1, true);
      WriteWString(L"⚫ Stopped");
    } else {
      SetColor(WHITE, -1, false);
      WriteWString(L"⚪ No Media");
    }
    ResetColor();

    if (!app.empty()) {
      std::wstring appDisplay = L"[" + app + L"]";
      size_t appPos = width_ - appDisplay.length() - 1;
      MoveCursor(STATUS_ROW, appPos);
      SetColor(CYAN, -1, false);
      WriteWString(appDisplay);
      ResetColor();
    }
    MoveCursor(STATUS_ROW, width_);
    WriteWString(L"│");
  }

  void TerminalUI::UpdateTrackInfo(const std::wstring& title, const std::wstring& artist, const std::wstring& album) {
    int maxWidth = width_ - 8;

    MoveCursor(TITLE_ROW, 3);
    ClearLine();
    SetColor(WHITE, -1, true);
    WriteWString(L"🎧 " + TruncateString(title, maxWidth - 3));
    ResetColor();
    MoveCursor(TITLE_ROW, width_);
    WriteWString(L"│");

    if (artist != L"") {
      MoveCursor(ARTIST_ROW, 3);
      ClearLine();
      SetColor(CYAN, -1, false);
      WriteWString(L"👤 " + TruncateString(artist, maxWidth - 3));
      ResetColor();
      MoveCursor(ARTIST_ROW, width_);
      WriteWString(L"│");
    } else {
      DrawEmptyLine(ARTIST_ROW);
    }

    if (album != L"") {
      MoveCursor(ALBUM_ROW, 3);
      ClearLine();
      SetColor(MAGENTA, -1, false);
      WriteWString(L"💿 " + TruncateString(album, maxWidth - 3));
      ResetColor();
      MoveCursor(ALBUM_ROW, width_);
      WriteWString(L"│");
    } else {
      DrawEmptyLine(ALBUM_ROW);
    }
  }

  void TerminalUI::UpdateProgress(long long currentSec, long long totalSec, bool isPlaying) {
    MoveCursor(PROGRESS_ROW, 3);
    ClearLine();

    int barWidth = width_ - 20;
    long long filled = (totalSec > 0) ? (currentSec * barWidth) / totalSec : 0;

    WriteWString(L"[");

    if (isPlaying) {
      SetColor(GREEN, -1, false);
    } else {
      SetColor(YELLOW, -1, false);
    }

    for (int i = 0; i < barWidth; i++) {
      if (i < filled) {
        WriteWString(L"█");
      } else if (i == filled) {
        WriteWString(L"▓");
      } else if (i == filled + 1) {
        WriteWString(L"▒");
      } else {
        WriteWString(L"░");
      }
    }

    ResetColor();
    WriteWString(L"] ");

    SetColor(BLACK, -1, false);
    WriteWString(FormatTime(currentSec) + L" / " + FormatTime(totalSec));

    ResetColor();
    WriteWString(L" │");
  }

  void TerminalUI::ShowLog(const std::string& json) {
    static const int LOG_START_COL = width_ + 2;
    static const int MAX_LOG_WIDTH = 80;

    // Clear previous log lines
    for (int i = HEADER_ROW; i <= PROGRESS_ROW + 1; i++) {
      MoveCursor(i, LOG_START_COL);
      ClearLine();
    }

    // Split JSON into lines of maximum width
    std::string remaining = json;
    int currentLine = HEADER_ROW;

    while (!remaining.empty() && currentLine <= PROGRESS_ROW + 1) {
      MoveCursor(currentLine, LOG_START_COL);
      SetColor(WHITE, -1, false);

      std::string chunk = remaining.substr(0, MAX_LOG_WIDTH);
      if (chunk.length() == MAX_LOG_WIDTH && remaining.length() > MAX_LOG_WIDTH) {
        chunk += "...";
      }

      WriteUTF8String(chunk);
      ResetColor();

      if (remaining.length() > MAX_LOG_WIDTH) {
        remaining = remaining.substr(MAX_LOG_WIDTH);
      } else {
        remaining.clear();
      }

      currentLine++;
    }
  }

  void TerminalUI::Clear() {
    WriteWString(L"\033[2J\033[H");
  }

  void TerminalUI::ShowCursor(bool show) {
    if (show) {
      ShowCursorInternal();
    } else {
      HideCursor();
    }
  }
} // namespace MonitorMedia