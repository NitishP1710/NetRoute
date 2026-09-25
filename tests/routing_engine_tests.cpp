#include "netroute/network.hpp"
#include "netroute/router.hpp"
#include "netroute/routing_engine.hpp"

#include <gtest/gtest.h>

#include <memory>

using namespace netroute;

namespace {

Network make_network() {
    Network network;

    network.add_router({"R1", "10.0.0.1"});
    network.add_router({"R2", "10.0.0.2"});
    network.add_router({"R3", "10.0.0.3"});
    network.add_router({"R4", "10.0.0.4"});

    network.add_link({"L12", "R1", "R2", 10.0, 1000.0, 0.1});
    network.add_link({"L13", "R1", "R3", 20.0, 500.0, 0.2});
    network.add_link({"L24", "R2", "R4", 15.0, 800.0, 0.1});
    network.add_link({"L34", "R3", "R4", 10.0, 900.0, 0.1});

    return network;
}

} // namespace

TEST(RoutingEngineTest, FindsLowestCostRoute) {
    auto network = make_network();
    RoutingEngine engine;

    const auto route = engine.find_route(network, "R1", "R4");

    ASSERT_TRUE(route.reachable());
    EXPECT_EQ(route.routers, std::vector<std::string>({"R1", "R2", "R4"}));
    EXPECT_NEAR(route.total_latency_ms, 25.0, 1e-9);
}

TEST(RoutingEngineTest, ReroutesAroundFailedLink) {
    auto network = make_network();
    RoutingEngine engine;

    network.fail_link("L24");
    const auto route = engine.find_route(network, "R1", "R4");

    ASSERT_TRUE(route.reachable());
    EXPECT_EQ(route.routers, std::vector<std::string>({"R1", "R3", "R4"}));
    EXPECT_NEAR(route.total_latency_ms, 30.0, 1e-9);
}

TEST(RoutingEngineTest, ReportsUnreachableDestination) {
    auto network = make_network();
    RoutingEngine engine;

    network.fail_link("L24");
    network.fail_link("L34");

    const auto route = engine.find_route(network, "R1", "R4");

    EXPECT_FALSE(route.reachable());
}

TEST(RoutingEngineTest, RecoveringLinkRestoresRoute) {
    auto network = make_network();
    RoutingEngine engine;

    network.fail_link("L24");
    network.recover_link("L24");

    const auto route = engine.find_route(network, "R1", "R4");

    ASSERT_TRUE(route.reachable());
    EXPECT_EQ(route.routers, std::vector<std::string>({"R1", "R2", "R4"}));
}

TEST(RoutingEngineTest, RejectsUnknownRouter) {
    auto network = make_network();
    RoutingEngine engine;

    EXPECT_THROW(
        engine.find_route(network, "R1", "RX"),
        std::invalid_argument);
}

TEST(RoutingEngineTest, SourceEqualsDestinationIsZeroCost) {
    auto network = make_network();
    RoutingEngine engine;

    const auto route = engine.find_route(network, "R2", "R2");

    ASSERT_TRUE(route.reachable());
    EXPECT_EQ(route.routers, std::vector<std::string>({"R2"}));
    EXPECT_DOUBLE_EQ(route.total_latency_ms, 0.0);
}

} // namespace
