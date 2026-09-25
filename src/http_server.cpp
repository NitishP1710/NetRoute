#include "netroute/http_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace netroute
{

    namespace
    {

        std::string read_file(const std::string &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("unable to open web asset: " + path);
            }
            std::ostringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        bool ends_with(const std::string &value, const std::string &suffix)
        {
            return value.size() >= suffix.size() &&
                   value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        std::string content_type(const std::string &path)
        {
            if (ends_with(path, ".html"))
                return "text/html; charset=utf-8";
            if (ends_with(path, ".js"))
                return "application/javascript; charset=utf-8";
            if (ends_with(path, ".css"))
                return "text/css; charset=utf-8";
            return "text/plain; charset=utf-8";
        }

        void send_response(int client, int status, const std::string &type,
                           const std::string &body)
        {
            const std::string status_text =
                status == 200 ? "OK" : status == 400 ? "Bad Request"
                                   : status == 404   ? "Not Found"
                                                     : "Internal Server Error";

            std::ostringstream response;
            response << "HTTP/1.1 " << status << " " << status_text << "\r\n"
                     << "Content-Type: " << type << "\r\n"
                     << "Content-Length: " << body.size() << "\r\n"
                     << "Connection: close\r\n"
                     << "Access-Control-Allow-Origin: *\r\n\r\n"
                     << body;

            const auto text = response.str();
            ::send(client, text.data(), text.size(), 0);
        }

        std::string json_escape(const std::string &value)
        {
            std::string result;
            for (const char c : value)
            {
                if (c == '"' || c == '\\')
                {
                    result += '\\';
                }
                result += c;
            }
            return result;
        }

        std::string state_json(const Network &network)
        {
            const auto ids = network.router_ids();
            std::ostringstream out;
            out << "{\"routers\":[";
            for (std::size_t i = 0; i < ids.size(); ++i)
            {
                if (i)
                    out << ',';
                const auto &router = network.router(ids[i]);
                out << "{\"id\":\"" << json_escape(router.id())
                    << "\",\"ip\":\"" << json_escape(router.ip_address()) << "\"}";
            }
            out << "],\"links\":[";
            const auto link_ids = network.link_ids();
            for (std::size_t i = 0; i < link_ids.size(); ++i)
            {
                if (i)
                    out << ',';
                const auto &link = network.link(link_ids[i]);
                out << "{\"id\":\"" << link.id()
                    << "\",\"source\":\"" << link.source()
                    << "\",\"destination\":\"" << link.destination()
                    << "\",\"latency\":" << link.latency_ms()
                    << ",\"bandwidth\":" << link.bandwidth_mbps()
                    << ",\"packetLoss\":" << link.packet_loss_percent()
                    << ",\"active\":" << (link.active() ? "true" : "false") << "}";
            }
            out << "]}";
            return out.str();
        }

        std::string route_json(const Route &route)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(2);
            if (!route.reachable())
            {
                return R"({"reachable":false,"routers":[]})";
            }

            out << "{\"reachable\":true,\"routers\":[";
            for (std::size_t i = 0; i < route.routers.size(); ++i)
            {
                if (i)
                    out << ',';
                out << "\"" << json_escape(route.routers[i]) << "\"";
            }
            out << "],\"cost\":" << route.total_cost
                << ",\"latency\":" << route.total_latency_ms
                << ",\"bandwidth\":" << route.minimum_bandwidth_mbps
                << ",\"packetLoss\":" << route.total_packet_loss_percent
                << "}";
            return out.str();
        }

        std::string query_value(const std::string &request, const std::string &key)
        {
            const auto marker = key + "=";
            const auto start = request.find(marker);
            if (start == std::string::npos)
                return {};
            const auto value_start = start + marker.size();
            const auto end = request.find_first_of(" &\r\n", value_start);
            return request.substr(value_start, end == std::string::npos ? std::string::npos : end - value_start);
        }

        std::string url_decode(const std::string &value)
        {
            std::string decoded;
            for (std::size_t i = 0; i < value.size(); ++i)
            {
                if (value[i] == '+')
                {
                    decoded += ' ';
                }
                else if (value[i] == '%' && i + 2 < value.size())
                {
                    const std::string hex = value.substr(i + 1, 2);
                    decoded += static_cast<char>(std::strtoul(hex.c_str(), nullptr, 16));
                    i += 2;
                }
                else
                {
                    decoded += value[i];
                }
            }
            return decoded;
        }

        std::string form_value(const std::string &body, const std::string &key)
        {
            const std::string marker = key + "=";
            const std::size_t start = body.find(marker);
            if (start == std::string::npos)
                return {};
            const std::size_t value_start = start + marker.size();
            const std::size_t end = body.find('&', value_start);
            return url_decode(body.substr(value_start,
                                          end == std::string::npos ? std::string::npos : end - value_start));
        }

        double form_number(const std::string &body, const std::string &key)
        {
            const std::string value = form_value(body, key);
            if (value.empty())
                throw std::invalid_argument("missing form field: " + key);
            return std::stod(value);
        }

        void handle_client(int client, Network &network, const RoutingEngine &engine)
        {
            char buffer[8192]{};
            const auto received = ::recv(client, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0)
            {
                ::close(client);
                return;
            }

            const std::string request(buffer, static_cast<std::size_t>(received));
            const std::size_t body_start = request.find("\r\n\r\n");
            const std::string body = body_start == std::string::npos
                                         ? std::string()
                                         : request.substr(body_start + 4);
            std::istringstream request_line(request);
            std::string method;
            std::string path;
            request_line >> method >> path;

            try
            {
                if (method == "GET" && (path == "/" || path == "/index.html"))
                {
                    send_response(client, 200, "text/html; charset=utf-8",
                                  read_file("web/index.html"));
                }
                else if (method == "GET" && path == "/app.js")
                {
                    send_response(client, 200, content_type(path), read_file("web/app.js"));
                }
                else if (method == "GET" && path == "/style.css")
                {
                    send_response(client, 200, content_type(path), read_file("web/style.css"));
                }
                else if (method == "GET" && path == "/api/state")
                {
                    send_response(client, 200, "application/json", state_json(network));
                }
                else if (method == "GET" && path.rfind("/api/route?", 0) == 0)
                {
                    const auto source = query_value(path, "source");
                    const auto destination = query_value(path, "destination");
                    send_response(client, 200, "application/json",
                                  route_json(engine.find_route(network, source, destination)));
                }
                else if (method == "POST" && path == "/api/routers")
                {
                    network.add_router(Router(form_value(body, "id"), form_value(body, "ip")));
                    send_response(client, 200, "application/json", state_json(network));
                }
                else if (method == "POST" && path == "/api/links")
                {
                    network.add_link(Link(
                        form_value(body, "id"),
                        form_value(body, "source"),
                        form_value(body, "destination"),
                        form_number(body, "latency"),
                        form_number(body, "bandwidth"),
                        form_number(body, "packetLoss")));
                    send_response(client, 200, "application/json", state_json(network));
                }
                else if (method == "POST" &&
                         path.rfind("/api/links/", 0) == 0)
                {
                    const auto remainder = path.substr(std::string("/api/links/").size());
                    const auto slash = remainder.find('/');
                    if (slash == std::string::npos)
                    {
                        send_response(client, 400, "text/plain", "invalid link operation");
                    }
                    else
                    {
                        const auto id = remainder.substr(0, slash);
                        const auto operation = remainder.substr(slash + 1);
                        if (operation == "fail")
                        {
                            network.fail_link(id);
                        }
                        else if (operation == "recover")
                        {
                            network.recover_link(id);
                        }
                        else
                        {
                            send_response(client, 400, "text/plain", "unknown link operation");
                            ::close(client);
                            return;
                        }
                        send_response(client, 200, "application/json", state_json(network));
                    }
                }
                else
                {
                    send_response(client, 404, "text/plain", "not found");
                }
            }
            catch (const std::exception &error)
            {
                send_response(client, 400, "text/plain", error.what());
            }

            ::close(client);
        }

    } // namespace

    HttpServer::HttpServer(Network &network, const RoutingEngine &engine, std::uint16_t port)
        : network_(network), engine_(engine), port_(port) {}

    void HttpServer::run()
    {
        const int server = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server < 0)
            throw std::runtime_error("failed to create socket");

        int reuse = 1;
        ::setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (::bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
        {
            ::close(server);
            throw std::runtime_error("failed to bind port " + std::to_string(port_));
        }

        if (::listen(server, 16) < 0)
        {
            ::close(server);
            throw std::runtime_error("failed to listen");
        }

        std::cout << "NetRoute UI: http://localhost:" << port_ << "\n";
        std::cout << "Press Ctrl+C to stop.\n";

        while (running_)
        {
            sockaddr_in client_address{};
            socklen_t client_size = sizeof(client_address);
            const int client = ::accept(
                server, reinterpret_cast<sockaddr *>(&client_address), &client_size);

            if (client >= 0)
            {
                handle_client(client, network_, engine_);
            }
        }

        ::close(server);
    }

    void HttpServer::stop() noexcept
    {
        running_ = false;
    }

} // namespace netroute
