#pragma once

#include "netroute/link.hpp"

namespace netroute {

class RouteCostStrategy {
public:
    virtual ~RouteCostStrategy() = default;
    virtual double cost(const Link& link) const = 0;
};

class BalancedCostStrategy final : public RouteCostStrategy {
public:
    double cost(const Link& link) const override;
};

} // namespace netroute
