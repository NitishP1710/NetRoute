#pragma once

#include <string>

namespace netroute {

class Router {
public:
    Router(std::string id, std::string ip_address);

    const std::string& id() const noexcept;
    const std::string& ip_address() const noexcept;

private:
    std::string id_;
    std::string ip_address_;
};

} // namespace netroute
