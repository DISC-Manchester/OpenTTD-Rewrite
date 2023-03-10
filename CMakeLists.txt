cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT BINARY_NAME)
    set(BINARY_NAME openttd)
endif()

project(${BINARY_NAME} VERSION 0.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please run \"cmake ..\" from the build directory. You may need to delete \"${CMAKE_SOURCE_DIR}/CMakeCache.txt\" first.")
endif()

# Debug mode by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DWITH_DEBUG)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/libs/include STATIC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include(LinkPackage)
    message(INFO " - building for Linux....")
    include(${PROJECT_SOURCE_DIR}/cmake/Linux.cmake)
    find_package(Vulkan)
    add_executable(openttd WIN32 ${PROJECT_SOURCE_DIR}/src/os/linux/Main.cpp)
    link_package(Vulkan)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(INFO " - building for Windows....")
    include(${PROJECT_SOURCE_DIR}/cmake/Windows.cmake)
    add_executable(openttd WIN32 ${PROJECT_SOURCE_DIR}/src/os/windows/Main.cpp)
    set_target_properties(openttd PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE" LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
else()
    message(FATAL_ERROR "system not supported")
endif()

include(SourceList)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

add_external_library(libos)

set_target_properties(openttd PROPERTIES OUTPUT_NAME "${BINARY_NAME}")
target_link_libraries(openttd Threads::Threads)
