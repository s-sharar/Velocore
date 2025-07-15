# Velocore - Chunk 1: Project Setup & Architecture

## Overview
A high-performance trading system simulator built with C++ and the Crow web framework. Velocore establishes a solid project foundation with a basic HTTP server and outlines the modular architecture for a complete trading system.

## Architecture

### Separation of Concerns
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   API Layer     │    │ Business Logic  │    │   Data Layer    │
│                 │    │                 │    │                 │
│ • HTTP Handling │───▶│ • Matching      │───▶│ • Order Struct  │
│ • JSON Parsing  │    │   Engine        │    │ • Trade Struct  │
│ • Crow Routes   │    │ • Order Book    │    │ • Data Models   │
│ • CORS/Auth     │    │ • Price-Time    │    │                 │
│                 │    │   Priority      │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Planned Components

1. **Data Models** (Next chunks)
   - `Order` struct: price, quantity, side, timestamp, etc.
   - `Trade` struct: execution records
   - Plain-old-data structures for performance

2. **Matching Engine** (Core component)
   - In-memory order book
   - Price-time priority matching
   - Buy/sell order management
   - Trade execution logic

3. **API Endpoints** (REST API)
   - Order submission: `POST /orders`
   - Order cancellation: `DELETE /orders/{id}`
   - Market data: `GET /orderbook`, `GET /trades`
   - System status: `GET /health`

4. **Concurrency & Safety**
   - Thread-safe operations (mutex/lock-free)
   - Concurrent client request handling
   - Atomic operations for critical sections

5. **Latency Simulation**
   - Per-client network delay simulation
   - Processing time hooks
   - Realistic trading environment

## Current Implementation (Chunk 1)

### Basic Crow Server
- ✅ HTTP server on port 18080
- ✅ Multithreaded request handling
- ✅ JSON response support
- ✅ Test endpoints for verification

### Available Endpoints
- `GET /ping` - Simple connectivity test
- `GET /health` - Server status with threading info
- `GET /architecture` - System design overview

## Setup Instructions

### 1. Dependencies (Automatic!)
Velocore uses CMake's FetchContent to automatically download Crow from GitHub if it's not found via package manager.

**Option A: Use package manager (faster builds):**
```bash
vcpkg install crow
```

**Option B: Automatic download (no setup required):**
Crow will be automatically fetched during build if not found locally.

### 2. Build Project
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 3. Run Server
```bash
./bin/Velocore
```

### 4. Test Endpoints
```bash
# Test connectivity
curl http://localhost:18080/ping

# Check server health  
curl http://localhost:18080/health

# View architecture
curl http://localhost:18080/architecture
```

## Expected Output
Server should start with output:
```
=== Velocore Trading Simulator ===
Initializing Crow web framework...
Starting server on port 18080
Available endpoints:
  GET /ping        - Simple ping/pong test
  GET /health      - Detailed health check
  GET /architecture - System architecture overview

Server running with multithreading enabled...
Hardware concurrency: X threads
```

## Technology Stack
- **Framework**: Crow (C++ HTTP framework)
- **Standard**: C++17 (threading, modern STL)
- **Build System**: CMake 3.15+
- **Threading**: Multi-threaded server with thread pool
- **JSON**: Crow's built-in JSON support

## Next Steps (Upcoming Chunks)
1. **Data Models**: Define Order/Trade structures
2. **Matching Engine**: Implement order book logic
3. **REST API**: Add trading endpoints
4. **Concurrency**: Thread safety mechanisms
5. **Performance**: Latency simulation and optimization

## Project Structure
```
Velocore/
├── CMakeLists.txt          # Build configuration with FetchContent
├── README.md              # This file
├── src/
│   └── main.cpp           # Basic Crow server
├── include/               # Headers (optional with FetchContent)
└── build/                 # Build output directory
```

---
**Status**: ✅ Chunk 1 Complete - Basic server running with architecture foundation 