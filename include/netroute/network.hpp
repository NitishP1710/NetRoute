#pragma once

#include "netroute/link.hpp"
#include "netroute/router.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace netroute {

class Network {
public:
    void add_router(Router router);
    void add_link(Link link);

    bool has_router(const std::string& id) const noexcept;
    bool has_link(const std::string& id) const noexcept;

    const Router& router(const std::string& id) const;
    Link& link(const std::string& id);
    const Link& link(const std::string& id) const;

    std::vector<const Link*> neighbors(const std::string& router_id) const;

    void fail_link(const std::string& link_id);
    void recover_link(const std::string& link_id);

    std::vector<std::string> router_ids() const;

private:
    std::unordered_map<std::string, Router> routers_;
    std::unordered_map<std::string, Link> links_;
    std::unordered_map<std::string, std::vector<std::string>> adjacency_;
};

} // namespace netroute
