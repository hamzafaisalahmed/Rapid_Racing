cmake_minimum_required(VERSION 3.21)
project(SpaceShooter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# SFML — use the pacman-installed copy (ucrt64 toolchain).
# Install first if you haven't:
#   pacman -S mingw-w64-ucrt-x86_64-sfml
# ============================================================
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

# ============================================================
# GAME EXECUTABLE
# ============================================================
add_executable(Racing
    src/main.cpp
    src/Game.cpp
)

target_link_libraries(Racing
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        opengl32
        winmm
        gdi32
)

target_compile_options(Racing PRIVATE -Wall)

# Release: hide the console window
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(Racing PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(Racing PRIVATE -mwindows)
endif()

# ============================================================
# FONT
# ============================================================

# 1. This ensures the 'assets' folder is copied every time you build
add_custom_command(TARGET Racing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Racing>/assets"
    COMMENT "Updating assets in build directory..."
)
