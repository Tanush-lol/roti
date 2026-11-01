cmake_minimum_required(VERSION 3.20)

project(roti LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)

add_executable(roti
    # Application
    src/main.c

    # Application scanner
    src/scanner/scanner.c

    # Wayland backend
    src/wayland/shm.c
    src/wayland/buffer.c
    src/wayland/xdg_surface.c

    # Cairo renderer
    src/wayland/cairo.c

    # Generated Wayland protocol implementation
    include/xdg-shell-protocol.c
)

target_include_directories(roti PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(roti PRIVATE
    PkgConfig::WAYLAND
    PkgConfig::CAIRO
    m
)

target_compile_options(roti PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
