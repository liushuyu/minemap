cmake_minimum_required (VERSION 3.0)
project (Minemap)
set (Minemap_VERSION_MAJOR 0)
set (Minemap_VERSION_MINOR 2)

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams filesystem)
pkg_check_modules(ImageMagick REQUIRED Magick++-7.Q16HDRI)

# Compilation options
set (CMAKE_CXX_STANDARD 17)
set(warnings "-g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

option(BUILD_PAMENIM "Build Pamenim" ON)

set(MINEMAP_PALETTE_DIR ${CMAKE_INSTALL_PREFIX}/share/minemap/palettes CACHE STRING "Directory to minemap palette GIFs")
add_compile_definitions(MINEMAP_PALETTE_DIR=\"${MINEMAP_PALETTE_DIR}\")

# External imports
option(NBTP_BUILD_PYTHON_MODULE OFF)
add_subdirectory(nbtp EXCLUDE_FROM_ALL)
target_include_directories(nbtp INTERFACE ./nbtp/include)

# Source files
add_subdirectory(src)

# Resources
add_subdirectory(misc)
