#include "netroute/router.hpp"

#include <utility>

using namespace std;

namespace netroute
{

    Router::Router(std::string id, std::string ip_address)
        : id_(std::move(id)), ip_address_(std::move(ip_address)) {}

    const std::string &Router::id() const noexcept { return id_; }
    const std::string &Router::ip_address() const noexcept { return ip_address_; }

} // namespace netroute
