cmake_minimum_required(VERSION 3.16)
project(VelocoreTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTEST QUIET gtest)
    pkg_check_modules(GTEST_MAIN QUIET gtest_main)
endif()

find_package(GTest QUIET)

if(NOT GTest_FOUND AND NOT GTEST_FOUND)
    find_path(GTEST_INCLUDE_DIR gtest/gtest.h
        PATHS /opt/homebrew/include /usr/local/include
        NO_DEFAULT_PATH)
    
    find_library(GTEST_LIBRARY gtest
        PATHS /opt/homebrew/lib /usr/local/lib
        NO_DEFAULT_PATH)
    
    find_library(GTEST_MAIN_LIBRARY gtest_main
        PATHS /opt/homebrew/lib /usr/local/lib
        NO_DEFAULT_PATH)
    
    if(GTEST_INCLUDE_DIR AND GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
        message(STATUS "Found GoogleTest manually: ${GTEST_INCLUDE_DIR}")
        set(GTEST_FOUND TRUE)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/models/include)

if(GTEST_INCLUDE_DIR)
    include_directories(${GTEST_INCLUDE_DIR})
endif()

add_executable(test_data_structures
    test_data_structures.cpp
    ../src/models/impl/Order.cpp
    ../src/models/impl/Trade.cpp
    ../src/models/impl/OrderBook.cpp
    ../src/models/impl/Types.cpp
)

if(GTest_FOUND)
    target_link_libraries(test_data_structures
        GTest::gtest
        GTest::gtest_main
        pthread
    )
elseif(GTEST_FOUND)
    target_link_libraries(test_data_structures
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        pthread
    )
elseif(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    target_link_libraries(test_data_structures
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
        pthread
    )
else()
    message(FATAL_ERROR "GoogleTest not found. Please install it with: brew install googletest")
endif()

enable_testing()
add_test(NAME DataStructuresTest COMMAND test_data_structures)

add_custom_target(run_unit_tests
    COMMAND ./test_data_structures
    DEPENDS test_data_structures
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_rigorous_chunk1-3.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(run_math_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_math_verification.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(run_api_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_velocore.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT TARGET GTest::gtest)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50f33f9ae90c96bc99e7bf8be6caa9.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()
 