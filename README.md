# NetRoute

A modern C++ network route planner and failure simulator.

NetRoute models routers and links as a weighted graph and computes the least-cost route between two routers. It can simulate link failures and automatically route around unavailable links.

## Features

- C++17 implementation with clear OOP boundaries
- Adjacency-list graph representation
- Dijkstra shortest-path routing
- Pluggable route-cost strategy
- Link failure/recovery simulation
- Route statistics: total latency, minimum bandwidth, packet-loss exposure
- JSON-free zero-dependency core library
- GoogleTest unit tests
- CMake build
- Interactive CLI demo

## Project structure

```
NetRoute/
├── CMakeLists.txt
├── include/netroute/
│   ├── link.hpp
│   ├── network.hpp
│   ├── route.hpp
│   ├── router.hpp
│   ├── routing_engine.hpp
│   └── routing_strategy.hpp
├── src/
│   ├── link.cpp
│   ├── network.cpp
│   ├── routing_engine.cpp
│   ├── routing_strategy.cpp
│   └── main.cpp
├── tests/
│   └── routing_engine_tests.cpp
└── .github/workflows/ci.yml
```

## Build

### Linux/macOS

```bash
cmake -S . -B build
cmake --build build -j
```

Run the demo:

```bash
./build/netroute
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

GoogleTest is downloaded automatically by CMake when tests are enabled.

## Example

Default topology:

```
R1 --10ms/1000Mbps-- R2 --15ms/800Mbps-- R4
 |                                  |
 +--20ms/500Mbps-- R3 --10ms/900Mbps+
```

The default route from R1 to R4 is:

```
R1 -> R2 -> R4
latency: 25 ms
```

After failing link R2-R4:

```
R1 -> R3 -> R4
latency: 30 ms
```

## Design notes

The graph is stored as an adjacency list, giving efficient neighbor traversal. Dijkstra's algorithm uses a min-priority queue and ignores inactive links.

The routing engine depends on a `RouteCostStrategy`, so the optimization policy can be changed without rewriting the graph traversal. The default strategy prioritizes latency while penalizing packet loss and low bandwidth.

This separation keeps the routing algorithm independent from business policy and gives the project a natural extension point for future heuristics.

## Future extensions

- REST API
- PostgreSQL-backed topology persistence
- network telemetry storage
- OSPF/BGP-inspired routing policies
- route caching
- concurrent telemetry ingestion
