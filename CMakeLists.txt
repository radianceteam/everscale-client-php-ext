cmake_minimum_required(VERSION 3.9)

set(PHP_SRC_ROOT_DIR build/phpmaster/vc15/x64/php-src)

include_directories(${PHP_SRC_ROOT_DIR})
include_directories(${PHP_SRC_ROOT_DIR}/main)
include_directories(${PHP_SRC_ROOT_DIR}/Zend)

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

set(CMAKE_PREFIX_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/deps
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/x64
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/x64)

project(ton-client-php-ext)

add_subdirectory(src)
