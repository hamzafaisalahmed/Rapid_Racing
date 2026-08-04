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


option(BUILD_TESTS "Build the unit test suite" ON)
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

option(BUILD_TESTS "Build the unit test suite" ON)

# ... game executable target as-is ...

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

    add_executable(unit_tests
        tests/test_utils.cpp
        tests/test_waypointhandler.cpp
        tests/test_collisions.cpp
        src/WaypointHandler.cpp
        src/AIController.cpp
        src/Car.cpp
        src/CollisionHandler.cpp
        src/TrackLoader.cpp
    )

    target_link_libraries(unit_tests
        PRIVATE
            Catch2::Catch2WithMain
            sfml-graphics
            sfml-window
            sfml-system
            sfml-audio
    )

    target_compile_options(unit_tests PRIVATE -Wall)

    add_custom_command(TARGET unit_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:unit_tests>/assets"
        COMMENT "Updating assets for unit tests..."
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(unit_tests)
endif()