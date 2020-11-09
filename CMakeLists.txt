cmake_minimum_required(VERSION 3.5)

project(ton_client
          VERSION 1.0.0
          LANGUAGES C)

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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
	add_definitions(-DHAVE_CONFIG_H)
	set(SOURCE_FILES "${SOURCE_FILES};config.h")
endif()

add_library(ton_client SHARED ${SOURCE_FILES})

message(STATUS "End cmaking of PHP extension")