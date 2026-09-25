#pragma once

#include "netroute/network.hpp"
#include "netroute/route.hpp"
#include "netroute/routing_strategy.hpp"

#include <memory>
#include <string>

namespace netroute {

class RoutingEngine {
public:
    explicit RoutingEngine(
        std::shared_ptr<const RouteCostStrategy> strategy =
            std::make_shared<BalancedCostStrategy>());

    Route find_route(const Network& network,
                     const std::string& source,
                     const std::string& destination) const;

private:
    std::shared_ptr<const RouteCostStrategy> strategy_;
};

} // namespace netroute
