#include "netroute/routing_engine.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>

using namespace std;

namespace netroute
{

    namespace
    {

        std::string other_endpoint(const Link &link, const std::string &current)
        {
            if (link.source() == current)
            {
                return link.destination();
            }
            if (link.destination() == current)
            {
                return link.source();
            }
            throw std::logic_error("link is not incident to current router");
        }

    } // namespace

    RoutingEngine::RoutingEngine(
        std::shared_ptr<const RouteCostStrategy> strategy)
        : strategy_(std::move(strategy))
    {
        if (!strategy_)
        {
            throw std::invalid_argument("routing strategy cannot be null");
        }
    }

    Route RoutingEngine::find_route(const Network &network,
                                    const std::string &source,
                                    const std::string &destination) const
    {
        if (!network.has_router(source) || !network.has_router(destination))
        {
            throw std::invalid_argument("source and destination routers must exist");
        }

        if (source == destination)
        {
            return {{source}, 0.0, 0.0, 0.0, 0.0};
        }

        using Entry = std::pair<double, std::string>;
        constexpr double infinity = std::numeric_limits<double>::infinity();

        std::unordered_map<std::string, double> distance;
        std::unordered_map<std::string, std::string> previous;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;

        for (const auto &id : network.router_ids())
        {
            distance[id] = infinity;
        }

        distance[source] = 0.0;
        queue.emplace(0.0, source);

        while (!queue.empty())
        {
            const Entry entry = queue.top();
            queue.pop();
            const double current_distance = entry.first;
            const std::string current = entry.second;

            if (current_distance > distance[current])
            {
                continue;
            }
            if (current == destination)
            {
                break;
            }

            for (const auto *link : network.neighbors(current))
            {
                const auto next = other_endpoint(*link, current);
                const auto candidate = current_distance + strategy_->cost(*link);

                if (candidate < distance[next])
                {
                    distance[next] = candidate;
                    previous[next] = current;
                    queue.emplace(candidate, next);
                }
            }
        }

        if (previous.find(destination) == previous.end())
        {
            return {};
        }

        std::vector<std::string> reversed;
        for (auto current = destination;; current = previous.at(current))
        {
            reversed.push_back(current);
            if (current == source)
            {
                break;
            }
        }

        std::reverse(reversed.begin(), reversed.end());

        Route route;
        route.routers = reversed;
        route.total_cost = distance[destination];
        route.minimum_bandwidth_mbps = std::numeric_limits<double>::infinity();

        for (std::size_t i = 1; i < route.routers.size(); ++i)
        {
            const auto &from = route.routers[i - 1];
            const auto &to = route.routers[i];

            const Link *selected = nullptr;
            for (const auto *link : network.neighbors(from))
            {
                const auto next = other_endpoint(*link, from);
                if (next == to)
                {
                    selected = link;
                    break;
                }
            }

            if (!selected)
            {
                throw std::logic_error("route reconstruction failed");
            }

            route.total_latency_ms += selected->latency_ms();
            route.minimum_bandwidth_mbps =
                std::min(route.minimum_bandwidth_mbps, selected->bandwidth_mbps());
            route.total_packet_loss_percent += selected->packet_loss_percent();
        }

        return route;
    }

} // namespace netroute
