cmake_minimum_required(VERSION 3.30)
project(MediaMonitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

add_executable(
    media_monitor_cli
    src/main.cpp
    src/media_session.cpp
    src/media_session.hpp
    src/settings.cpp
    src/settings.hpp
    src/terminal_ui.cpp
    src/terminal_ui.hpp
    src/track_info.hpp
    src/websocket_server.cpp
    src/websocket_server.hpp
    src/websocket_session.cpp
    src/websocket_session.hpp
)

add_compile_definitions(UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

target_compile_options(media_monitor_cli PRIVATE /Ox /Ob2 /MP /W4 /WX /utf-8 /EHsc /MDd)
target_link_libraries(media_monitor_cli
    PRIVATE
    Boost::system
    nlohmann_json::nlohmann_json
    windowsapp
)
