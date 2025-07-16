cmake_minimum_required(VERSION 3.15)
project(Velocore VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

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

set(SOURCES
    src/main.cpp
    src/models/impl/Types.cpp
    src/models/impl/Order.cpp
    src/models/impl/Trade.cpp
)

set(HEADERS
    src/models/include/Types.h
    src/models/include/Order.h
    src/models/include/Trade.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Threads::Threads
)

if(Crow_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Crow::Crow)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Crow::crow)
endif()

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

message(STATUS "=== Velocore Trading Simulator Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "===============================================")
