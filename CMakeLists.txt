cmake_minimum_required(VERSION 3.21)
project(Rapid_Racing LANGUAGES CXX)

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
add_executable(Rapid_Racing
    src/main.cpp
    src/Game.cpp
    src/Graphics.cpp
    src/CollisionHandler.cpp
    src/AIController.cpp
    src/Car.cpp
    src/StateManager.cpp
    src/WaypointHandler.cpp
    src/TrackLoader.cpp
    app.rc
    icon.ico
)

target_link_libraries(Rapid_Racing
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        opengl32
        winmm
        gdi32
)

target_compile_options(Rapid_Racing PRIVATE -Wall)

# Release: hide the console window
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(Rapid_Racing PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(Rapid_Racing PRIVATE -mwindows)
endif()

# ============================================================
# FONT
# ============================================================

# 1. This ensures the 'assets' folder is copied every time you build
add_custom_command(TARGET Rapid_Racing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Rapid_Racing>/assets"
    COMMENT "Updating assets in build directory..."
)

# ============================================================
# CATCH2 AUTOMATED TESTS
# ============================================================

# 1. Fetch Catch2 v3 automatically from GitHub
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# 2. Allow test files to #include "WaypointHandler.h" directly
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# 3. Test Executable Target
# Note: Include game .cpp logic files needed by tests, but EXCLUDE src/main.cpp!
add_executable(unit_tests
    tests/test_utils.cpp      
    tests/test_waypointhandler.cpp
    src/WaypointHandler.cpp
    src/AIController.cpp
    src/Car.cpp
    src/CollisionHandler.cpp
    src/TrackLoader.cpp
)

# 4. Link Catch2's main runner and SFML
target_link_libraries(unit_tests
    PRIVATE
        Catch2::Catch2WithMain
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)

target_compile_options(unit_tests PRIVATE -Wall)

# 5. Copy assets folder so tests can load track JSONs or textures if needed
add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:unit_tests>/assets"
    COMMENT "Updating assets for unit tests..."
)

# 6. Enable CTest support for automated test discovery
include(CTest)
include(Catch)
catch_discover_tests(unit_tests)