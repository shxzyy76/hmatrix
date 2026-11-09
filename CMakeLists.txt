cmake_minimum_required(VERSION 3.10)
project(hmatrix VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CURSES_NEED_WIDE TRUE)  # wide-character ncurses for UTF-8 chars
find_package(Curses REQUIRED)

# TOML parser: vendored tomlc17 (MIT), built without our strict warnings.
add_library(tomlc17 STATIC third_party/tomlc17/tomlc17.c)
target_include_directories(tomlc17 PUBLIC third_party/tomlc17)
target_compile_definitions(tomlc17 PRIVATE _POSIX_C_SOURCE=200809L)

add_executable(hmatrix
  src/main.c src/animation.c src/renderer.c src/config.c src/cli.c)
target_include_directories(hmatrix PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(hmatrix PRIVATE tomlc17 ${CURSES_LIBRARIES})
target_compile_options(hmatrix PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(hmatrix PRIVATE
  _XOPEN_SOURCE=700 HMATRIX_VERSION="${PROJECT_VERSION}")

install(TARGETS hmatrix RUNTIME DESTINATION bin)
install(FILES config/hmatrix.toml DESTINATION share/hmatrix)
