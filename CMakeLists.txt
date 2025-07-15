cmake_minimum_required(VERSION 3.15)
project(Velocore VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(Threads REQUIRED)

find_package(Crow CONFIG QUIET)

if(NOT Crow_FOUND)
    message(STATUS "Crow not found via package manager, fetching from GitHub...")
    
    FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
    )
    
    FetchContent_MakeAvailable(crow)
    
    set(CROW_INCLUDE_DIR ${crow_SOURCE_DIR}/include)
    set(CROW_FETCHED TRUE)
    
    message(STATUS "Crow fetched successfully via FetchContent")
    message(STATUS "Crow include directory: ${CROW_INCLUDE_DIR}")
endif()

set(SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

if(Crow_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Crow::Crow)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Crow::crow)
    target_include_directories(${PROJECT_NAME} PRIVATE ${crow_SOURCE_DIR}/include)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE wsock32 ws2_32)
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
endif()

message(STATUS "Velocore Trading Simulator configured for build")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
