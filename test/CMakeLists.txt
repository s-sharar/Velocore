cmake_minimum_required(VERSION 3.16)
project(VelocoreTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTEST QUIET gtest)
    pkg_check_modules(GTEST_MAIN QUIET gtest_main)
endif()

find_package(GTest QUIET)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

include(FetchContent)

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

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/models/include)
# Crow and nlohmann_json include dirs are provided by their targets when linked

if(GTEST_INCLUDE_DIR)
    include_directories(${GTEST_INCLUDE_DIR})
endif()

# Original data structures test
add_executable(test_data_structures
    test_data_structures.cpp
    ../src/models/impl/Order.cpp
    ../src/models/impl/Trade.cpp
    ../src/models/impl/OrderBook.cpp
    ../src/models/impl/OrderBookRegistry.cpp
    ../src/models/impl/MarketTickStore.cpp
    ../src/models/impl/Types.cpp
)

# New market data test
add_executable(test_market_data
    test_market_data.cpp
    ../src/models/impl/Order.cpp
    ../src/models/impl/Trade.cpp
    ../src/models/impl/OrderBook.cpp
    ../src/models/impl/Types.cpp
    ../src/MarketDataFeed.cpp
)

# WebSocket parsing test
add_executable(test_websocket_parsing
    test_websocket_parsing.cpp
    ../src/models/impl/Order.cpp
    ../src/models/impl/Trade.cpp
    ../src/models/impl/OrderBook.cpp
    ../src/models/impl/Types.cpp
    ../src/MarketDataFeed.cpp
)

# Alpaca broker tests (dry-run)
add_executable(test_alpaca_broker
    test_alpaca_broker.cpp
)

# Link libraries for data structures test
if(GTest_FOUND)
    target_link_libraries(test_data_structures
        GTest::gtest
        GTest::gtest_main
    )
elseif(GTEST_FOUND)
    target_link_libraries(test_data_structures
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
    )
elseif(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    target_link_libraries(test_data_structures
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
    )
else()
    # Fetch GoogleTest if not available
    if(NOT TARGET GTest::gtest)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50f33f9ae90c96bc99e7bf8be6caa9.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    target_link_libraries(test_websocket_parsing
        GTest::gtest
        GTest::gtest_main
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    )
endif()

# Link libraries for Alpaca broker test
if(GTest_FOUND)
    target_link_libraries(test_alpaca_broker
        GTest::gtest
        GTest::gtest_main
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        brokers
    )
elseif(GTEST_FOUND)
    target_link_libraries(test_alpaca_broker
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        brokers
    )
elseif(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    target_link_libraries(test_alpaca_broker
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        brokers
    )
else()
    if(NOT TARGET GTest::gtest)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50f33f9ae90c96bc99e7bf8be6caa9.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    target_link_libraries(test_alpaca_broker
        GTest::gtest
        GTest::gtest_main
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        brokers
        pthread
    )
endif()

# Link libraries for market data test
if(GTest_FOUND)
    target_link_libraries(test_market_data
        GTest::gtest
        GTest::gtest_main
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
elseif(GTEST_FOUND)
    target_link_libraries(test_market_data
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
elseif(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    target_link_libraries(test_market_data
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
else()
    message(FATAL_ERROR "GoogleTest not found. Please install it with: brew install googletest")
endif()

# Link libraries for WebSocket parsing test
if(GTest_FOUND)
    target_link_libraries(test_websocket_parsing
        GTest::gtest
        GTest::gtest_main
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
elseif(GTEST_FOUND)
    target_link_libraries(test_websocket_parsing
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
elseif(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
    target_link_libraries(test_websocket_parsing
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
    )
else()
    message(FATAL_ERROR "GoogleTest not found. Please install it with: brew install googletest")
endif()

# Enable testing
enable_testing()
add_test(NAME DataStructuresTest COMMAND test_data_structures)
add_test(NAME MarketDataTest COMMAND test_market_data)
add_test(NAME WebSocketParsingTest COMMAND test_websocket_parsing)
add_test(NAME AlpacaBrokerTest COMMAND test_alpaca_broker)

# Custom targets
add_custom_target(run_unit_tests
    COMMAND $<TARGET_FILE:test_data_structures>
    COMMAND $<TARGET_FILE:test_market_data>
    COMMAND $<TARGET_FILE:test_websocket_parsing>
    COMMAND $<TARGET_FILE:test_alpaca_broker>
    DEPENDS test_data_structures test_market_data test_websocket_parsing test_alpaca_broker
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_rigorous.sh
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

# Market data specific test targets
add_custom_target(run_market_data_tests
    COMMAND ./test_market_data
    DEPENDS test_market_data
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_websocket_tests
    COMMAND ./test_websocket_parsing
    DEPENDS test_websocket_parsing
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(run_all_market_tests
    COMMAND ./test_market_data
    COMMAND ./test_websocket_parsing
    DEPENDS test_market_data test_websocket_parsing
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT TARGET GTest::gtest)
    FetchContent_Declare(
        googletest_fallback
        URL https://github.com/google/googletest/archive/03597a01ee50f33f9ae90c96bc99e7bf8be6caa9.zip
    )
    FetchContent_MakeAvailable(googletest_fallback)
endif()
 