# Media Monitor

A lightweight Windows application that monitors currently playing media across different apps and exposes the information via a WebSocket server.

> This app started as a study on Windows C++ API's.
> The goal is to enhance this with other functionalities that requires audio/media API's in Windows.

## AI usage

Code is not written by AI and it has been used as guidance and overall project management, like:
- Documentation
- Sample files

## Features

- Monitors active media sessions on Windows using the Windows Runtime Media API
- Tracks media information including:
  - Title
  - Artist
  - Album
  - Duration
  - Playback state (playing/paused)
  - Thumbnail
- Exposes real-time media updates via WebSocket server
- Command-line interface for monitoring and control
- JSON-based configuration and data format

## Requirements

- Windows 10 or later
- Visual Studio 2022 Build Tools or Visual Studio 2022
- CMake 3.30 or later
- vcpkg package manager

## Dependencies

- Boost.System (for WebSocket server)
- nlohmann_json (for JSON handling)
- Windows Runtime APIs

## Building

1. Clone the repository
2. Make sure vcpkg is properly set up and integrated with CMake
3. Build using CMake:

```sh
# Debug build
cmake --build build --config Debug

# Release build
cmake --build build --config Release
```

## Running

After building, run the CLI executable:

```sh
./build/Debug/media_monitor_cli.exe    # Debug build
./build/Release/media_monitor_cli.exe  # Release build
```

Command line options:
- `-l | --show-logs`: Logs the last JSON broadcasted

## WebSocket API

The application starts a WebSocket server that clients can connect to for receiving real-time media updates. Media information is sent as JSON messages in the following format:

```json
{
  "title": "Song Title",
  "artist": "Artist Name",
  "album": "Album Name",
  "duration": 180,
  "state": "playing",
  "thumbnail": "base64"
}
```

Connect to the WebSocket server at: `ws://localhost:PORT` (default port can be configured in settings)

## Configuration

Settings can be configured by modifying the application's settings file.

**Partially implemented**

## Roadmap

> no commitment, may change at any time

- Release workflow
- Settings
- Tests
- Custom Windows User Interface
- Audio Sessions detection
- Custom scripting (integrate with Node.JS(?) NAPI)
- Audio manipulation (play, stop, mute, volume)
