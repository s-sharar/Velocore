# Velocore Trading Simulator

Welcome to Velocore! This project is a high-performance trading system simulator built from the ground up in modern C++. It's designed to be a fast, robust, and extensible foundation for simulating a real-world electronic trading environment.

At its core, Velocore provides a multithreaded web server that exposes a REST API for interacting with the trading system.

## ✨ Features

*   **RESTful API**: A clean, easy-to-use API for submitting orders, logging trades, and checking system status.
*   **Core Data Models**: Well-defined C++ structures for `Order` and `Trade` objects, complete with business logic for tracking state and serializing to JSON.
*   **High-Performance Foundation**: Built with modern C++ (17) and a multithreaded architecture to handle concurrent requests efficiently.
*   **Modern Build System**: Uses CMake for easy, cross-platform configuration and building.
*   **Clean Architecture**: A clear separation between the API layer and the underlying data models, making the system easy to extend and maintain.

## 🚀 Getting Started

 Follow these steps to get the server up and running on your machine.

### Prerequisites

*   A C++ compiler that supports C++17 (like Clang or GCC)
*   CMake (version 3.15 or higher)
*   `curl` and `jq` for testing the API from the command line

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone <repo-url>
    cd Velocore
    ```

2.  **Configure and build with CMake:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
    This will compile the project and place the executable in the `build/bin` directory. The Crow dependency is fetched automatically if not found on your system.

3.  **Run the Server:**
    ```bash
    ./bin/Velocore
    ```
    You should see a confirmation that the server is running on `http://0.0.0.0:18080`.

## 🧪 Using the API

Once the server is running, open a **new terminal window** to interact with the API using `curl`.

### Check Server Health

A simple ping to see if the server is alive.

```bash
curl http://localhost:18080/ping
# Expected Response: {"message":"pong"}
```

### Create an Order

Submit a new limit order to the system.

```bash
curl -X POST http://localhost:18080/orders \
  -H "Content-Type: application/json" \
  -d '{
    "symbol": "SIM",
    "side": "BUY",
    "type": "LIMIT",
    "price": 100.50,
    "quantity": 100
  }' | jq
```

### View All Orders

Get a list of all orders currently in the system.

```bash
curl http://localhost:18080/orders | jq
```

### View System Statistics

Check out real-time statistics, including total orders and trade volume.

```bash
curl http://localhost:18080/statistics | jq
```

## 🛠️ Tech Stack

*   **C++17**: For modern, efficient, and robust code.
*   **Crow (C++ Micro Web Framework)**: For the lightweight and fast REST API server.
*   **CMake**: For cross-platform build automation.
