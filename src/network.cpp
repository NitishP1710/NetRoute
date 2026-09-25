#include "netroute/network.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace netroute {

void Network::add_router(Router router) {
    if (router.id().empty()) {
        throw std::invalid_argument("router id cannot be empty");
    }
    auto [it, inserted] = routers_.emplace(router.id(), std::move(router));
    if (!inserted) {
        throw std::invalid_argument("router already exists: " + it->first);
    }
    adjacency_[it->first];
}

void Network::add_link(Link link) {
    if (!has_router(link.source()) || !has_router(link.destination())) {
        throw std::invalid_argument("link endpoints must exist");
    }
    if (has_link(link.id())) {
        throw std::invalid_argument("link already exists: " + link.id());
    }

    const auto source = link.source();
    const auto destination = link.destination();
    const auto id = link.id();

    links_.emplace(id, std::move(link));
    adjacency_[source].push_back(id);
    adjacency_[destination].push_back(id);
}

bool Network::has_router(const std::string& id) const noexcept {
    return routers_.find(id) != routers_.end();
}

bool Network::has_link(const std::string& id) const noexcept {
    return links_.find(id) != links_.end();
}

const Router& Network::router(const std::string& id) const {
    const auto it = routers_.find(id);
    if (it == routers_.end()) {
        throw std::out_of_range("unknown router: " + id);
    }
    return it->second;
}

Link& Network::link(const std::string& id) {
    const auto it = links_.find(id);
    if (it == links_.end()) {
        throw std::out_of_range("unknown link: " + id);
    }
    return it->second;
}

const Link& Network::link(const std::string& id) const {
    const auto it = links_.find(id);
    if (it == links_.end()) {
        throw std::out_of_range("unknown link: " + id);
    }
    return it->second;
}

std::vector<const Link*> Network::neighbors(const std::string& router_id) const {
    if (!has_router(router_id)) {
        throw std::out_of_range("unknown router: " + router_id);
    }

    std::vector<const Link*> result;
    for (const auto& id : adjacency_.at(router_id)) {
        const auto& candidate = links_.at(id);
        if (candidate.active()) {
            result.push_back(&candidate);
        }
    }
    return result;
}

void Network::fail_link(const std::string& link_id) {
    link(link_id).fail();
}

void Network::recover_link(const std::string& link_id) {
    link(link_id).recover();
}

std::vector<std::string> Network::router_ids() const {
    std::vector<std::string> ids;
    ids.reserve(routers_.size());
    for (const auto& [id, _] : routers_) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

} // namespace netroute
