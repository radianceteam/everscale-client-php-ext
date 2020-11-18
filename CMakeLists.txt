cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

set(CMAKE_PREFIX_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/deps
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/x64
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/x64)

project(ton-client-php-ext)

add_subdirectory(src)
