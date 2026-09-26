# NetRoute

A modern C++ network route planner and failure simulator with a small browser UI backed directly by the C++ routing engine.

## What it demonstrates

- C++14 and object-oriented design
- Graph data structures and adjacency lists
- Dijkstra shortest-path routing
- Strategy pattern for route-cost policies
- Link failure/recovery and automatic rerouting
- Live topology editing: add routers and links from the browser
- Failure controls generated from the current topology, including newly added links
- Route metrics: latency, bandwidth, packet loss and cost
- Lightweight HTTP server written in C++
- Browser UI connected to the C++ backend through REST-like HTTP endpoints
- CMake build system

## Architecture

```
Browser UI
   │
   │ HTTP / JSON
   ▼
C++ HTTP Server
   │
   ▼
Network + RoutingEngine
   │
   ├── Graph / adjacency list
   ├── Dijkstra
   └── RouteCostStrategy
```

There is no separate frontend server and no Docker requirement. The C++ executable serves the static UI and the API from the same process.

## Run

### Linux/macOS

```bash
cmake -S . -B build
cmake --build build -j
./build/netroute
```

Open:

```
http://localhost:8080
```

You can change the port:

```bash
NETROUTE_PORT=9090 ./build/netroute
```

## UI capabilities

- Select source and destination routers
- Calculate the current optimal route
- View latency, bandwidth, cost and packet loss
- Fail a link from the UI
- Recover a link from the UI
- Immediately recompute the route
- See current topology/link state
- Add a router by providing its ID and IP address
- Add a link by providing its endpoints and latency, bandwidth and packet-loss metrics

Topology changes are held in memory by the running C++ process. Restarting the
program restores the demo topology from `src/main.cpp`.

## API

```
GET  /api/state
GET  /api/route?source=R1&destination=R4
POST /api/routers
POST /api/links
POST /api/links/L24/fail
POST /api/links/L24/recover
```

`POST /api/routers` accepts URL-encoded fields:

```text
id=R5&ip=10.0.0.5
```

`POST /api/links` accepts URL-encoded fields:

```text
id=L45&source=R4&destination=R5&latency=12&bandwidth=700&packetLoss=0.2
```

The API operates on the same in-memory `Network` object used by the routing
engine. Every state response enumerates the current routers and links, so a
newly added link immediately appears in the failure list and is used by
Dijkstra after it is failed or recovered.

## Example

Initial:

```
R1 -> R2 -> R4
```

Add router `R5` and link `L45` from `R4` to `R5`, then fail `L24` in the UI:

```
R1 -> R3 -> R4
```

Recover it and the original route becomes available again. The same controls
work for `L45` or any other link added during the session.

## Project structure

```
NetRoute/
├── include/netroute/
│   ├── http_server.hpp
│   ├── link.hpp
│   ├── network.hpp
│   ├── route.hpp
│   ├── router.hpp
│   ├── routing_engine.hpp
│   └── routing_strategy.hpp
├── src/
│   ├── http_server.cpp
│   ├── link.cpp
│   ├── main.cpp
│   ├── network.cpp
│   ├── router.cpp
│   ├── routing_engine.cpp
│   └── routing_strategy.cpp
├── tests/
├── web/
│   ├── app.js
│   ├── index.html
│   └── style.css
└── CMakeLists.txt
```

## Why the UI is useful

The UI is intentionally small. The project is primarily a C++ routing-engine project, while the browser provides a visual way to demonstrate the graph, route computation, and failure recovery during an interview.

## Future extensions

- PostgreSQL topology persistence
- network telemetry
- route caching
- OSPF/BGP-inspired routing policies
- concurrent request handling
- richer topology visualization
