cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(Velocore VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows-specific definitions
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0B00)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

if(MSVC)
    add_compile_options(/bigobj)
    add_compile_options(/permissive-)
endif()

option(BUILD_TESTING "Build the tests" OFF)

find_package(Threads REQUIRED)

find_package(Boost REQUIRED CONFIG COMPONENTS system)

find_package(OpenSSL REQUIRED)

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "nlohmann_json fetched successfully")
endif()

find_package(Crow CONFIG QUIET)

if(NOT Crow_FOUND)
    message(STATUS "Crow not found via package manager, fetching from GitHub...")
    
    include(FetchContent)
    FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(crow)
    message(STATUS "Crow fetched successfully")
endif()

add_subdirectory(src/models)

set(SOURCES
    src/main.cpp
    src/MarketDataFeed.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Threads::Threads
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    models
)


if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic -O2
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

message(STATUS "=== Velocore Trading Simulator Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "===============================================")
