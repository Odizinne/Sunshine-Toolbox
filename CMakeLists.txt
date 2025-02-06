cmake_minimum_required(VERSION 3.23)
project(Sunshine-Toolbox VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
include_directories(SteamWindowManager)
set(SOURCES
    SteamWindowManager/steamwindowmanager.cpp
    main.cpp
)
set(HEADERS
    SteamWindowManager/steamwindowmanager.h
)
add_executable(Sunshine-Toolbox ${SOURCES} ${HEADERS})
target_link_libraries(Sunshine-Toolbox
    PRIVATE
        User32
        Shell32
        Advapi32
)
