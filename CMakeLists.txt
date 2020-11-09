# Note: This file is needed for IDE support only.
# We still have to build PHP extension using PHP SDK tools,
# especially on Windows.

cmake_minimum_required(VERSION 3.5)

set(TC_VERSION_MAJOR 1)
set(TC_VERSION_MINOR 1)
set(TC_VERSION_PATCH 0)
set(TC_VERSION_SUFFIX "-rc")
set(TC_VERSION_STRING ${TC_VERSION_MAJOR}.${TC_VERSION_MINOR}.${TC_VERSION_PATCH})
set(TC_VERSION_FULL ${TC_VERSION_STRING}${TC_VERSION_SUFFIX})

project(ton_client VERSION ${TC_VERSION_STRING} LANGUAGES C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

message(STATUS "Begin cmaking of PHP extension ...")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

if (NOT CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 99)
endif ()

set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0 -Wall -std=gnu99 -fvisibility=hidden")

set(PHP_VERSION 7.4.1)
set(PHP_SRC_PATH "../..")

list(APPEND CMAKE_PREFIX_PATH "${PHP_SRC_PATH}")

include_directories(
        "${PHP_SRC_PATH}"
        "${PHP_SRC_PATH}/main"
        "${PHP_SRC_PATH}/Zend"
        "${PHP_SRC_PATH}/TSRM"
        "${PHP_SRC_PATH}/ext"
        "${PHP_SRC_PATH}/sapi")

FILE(GLOB KernelHeaders ./kernel/*.h)
FILE(GLOB KernelSources ./kernel/*.c)

set(SOURCE_FILES
        ton_client.c
        php_ton_client.h
        ${KernelHeaders}
        ${KernelSources})

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    add_definitions(-DHAVE_CONFIG_H)
    set(SOURCE_FILES "${SOURCE_FILES};config.h")
endif ()

find_package(TON REQUIRED)

add_library(ton_client SHARED ${SOURCE_FILES})

set_target_properties(ton_client PROPERTIES
        C_VISIBILITY_PRESET hidden
        COMPILE_DEFINITIONS "PHP_WIN32"
        VERSION ${TC_VERSION_STRING}
        SOVERSION ${TC_VERSION_MAJOR})

target_link_libraries(ton_client TON::Client)

message(STATUS "End cmaking of PHP extension")
