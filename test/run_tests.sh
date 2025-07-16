#!/bin/bash

# Velocore Trading Engine Test Runner
# This script builds and runs all tests for the WebSocket Market Data Feed

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "$1 is not installed or not in PATH"
        exit 1
    fi
}

# Function to run a test and capture results
run_test() {
    local test_name="$1"
    local test_executable="$2"
    
    print_status "Running $test_name..."
    
    if ./"$test_executable" > "${test_name}.log" 2>&1; then
        print_success "$test_name PASSED"
        return 0
    else
        print_error "$test_name FAILED"
        echo "Error output:"
        cat "${test_name}.log"
        return 1
    fi
}

# Main function
main() {
    print_status "Starting Velocore Trading Engine Test Runner"
    
    # Check prerequisites
    print_status "Checking prerequisites..."
    check_command "cmake"
    check_command "make"
    check_command "g++"
    
    # Set up environment
    export ALPACA_API_KEY="test_key_123"
    export ALPACA_API_SECRET="test_secret_456"
    export ALPACA_BASE_URL="https://paper-api.alpaca.markets"
    export ALPACA_DATA_URL="wss://stream.data.alpaca.markets/v2/iex"
    export ALPACA_PAPER_TRADING="true"
    
    # Create build directory
    print_status "Setting up build directory..."
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    # Configure CMake
    print_status "Configuring CMake..."
    if ! cmake .. > cmake.log 2>&1; then
        print_error "CMake configuration failed"
        cat cmake.log
        exit 1
    fi
    
    # Build all tests
    print_status "Building all tests..."
    if ! make -j$(nproc 2>/dev/null || echo 4) > build.log 2>&1; then
        print_error "Build failed"
        cat build.log
        exit 1
    fi
    
    print_success "All tests built successfully"
    
    # Run tests
    print_status "Running test suite..."
    
    failed_tests=0
    total_tests=0
    
    # Test 1: Data Structures Test
    total_tests=$((total_tests + 1))
    if ! run_test "DataStructuresTest" "test_data_structures"; then
        failed_tests=$((failed_tests + 1))
    fi
    
    # Test 2: Market Data Test
    total_tests=$((total_tests + 1))
    if ! run_test "MarketDataTest" "test_market_data"; then
        failed_tests=$((failed_tests + 1))
    fi
    
    # Test 3: WebSocket Parsing Test
    total_tests=$((total_tests + 1))
    if ! run_test "WebSocketParsingTest" "test_websocket_parsing"; then
        failed_tests=$((failed_tests + 1))
    fi
    
    # Summary
    echo
    echo "========================================"
    echo "           TEST SUMMARY"
    echo "========================================"
    echo "Total tests run: $total_tests"
    echo "Passed: $((total_tests - failed_tests))"
    echo "Failed: $failed_tests"
    echo
    
    if [ $failed_tests -eq 0 ]; then
        print_success "All tests passed! ✅"
        exit 0
    else
        print_error "Some tests failed! ❌"
        exit 1
    fi
}

# Help function
show_help() {
    echo "Velocore Trading Engine Test Runner"
    echo
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -v, --verbose  Enable verbose output"
    echo "  -c, --clean    Clean build directory before running"
    echo "  -t, --test     Run specific test (data|market|websocket)"
    echo
    echo "Examples:"
    echo "  $0                    # Run all tests"
    echo "  $0 --clean           # Clean build and run all tests"
    echo "  $0 --test market     # Run only market data tests"
    echo "  $0 --verbose         # Run with verbose output"
}

# Parse command line arguments
VERBOSE=false
CLEAN=false
SPECIFIC_TEST=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -t|--test)
            SPECIFIC_TEST="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    print_status "Cleaning build directory..."
    rm -rf build
fi

# Enable verbose output if requested
if [ "$VERBOSE" = true ]; then
    set -x
fi

# Run specific test if requested
if [ -n "$SPECIFIC_TEST" ]; then
    print_status "Running specific test: $SPECIFIC_TEST"
    
    # Set up build environment
    if [ ! -d "build" ]; then
        mkdir build
    fi
    cd build
    
    # Configure and build
    cmake .. > /dev/null 2>&1
    make -j$(nproc 2>/dev/null || echo 4) > /dev/null 2>&1
    
    case $SPECIFIC_TEST in
        data)
            run_test "DataStructuresTest" "test_data_structures"
            ;;
        market)
            run_test "MarketDataTest" "test_market_data"
            ;;
        websocket)
            run_test "WebSocketParsingTest" "test_websocket_parsing"
            ;;
        *)
            print_error "Unknown test: $SPECIFIC_TEST"
            print_error "Available tests: data, market, websocket"
            exit 1
            ;;
    esac
    exit 0
fi

# Run main function
main "$@" 