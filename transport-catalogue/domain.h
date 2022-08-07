#pragma once

#include <string>
#include <vector>
#include <variant>
#include <set>

#include "geo.h"

struct Stop {
    std::string name;
    geo::Coordinates position;
};
using StopPtr = const Stop*;

struct Bus {
    std::string name;
    std::vector<StopPtr> stops;
    bool is_roundtrip = false;
};
using BusPtr = const Bus*;

struct BusStat {
    size_t stops_count = 0u;
    size_t unique_stops = 0u;
    double route_length = 0.;
    double curvature = 0.;
};
struct StopStat {
    bool is_found = false;
    std::set<std::string_view> data;
};

namespace detail {

    struct DistanceHasher {
        template <class T>
        std::size_t operator()(const std::pair<T, T>& pair) const {
            return std::hash<T>()(pair.first) + 37 * std::hash<T>()(pair.second);
        }
    };

} // namespace detail

namespace route {

    enum class RouteType {
        WAIT,
        BUS
    };

    struct RouteSettings {
        double bus_wait_time = 0.0;
        double bus_velocity = 0.0;
    };

    struct RouteItemStat {
        RouteType type = RouteType::BUS;
        std::string_view name;
        int span_cout = 0;
        double time = 0.0;
    };

    struct RouteStat {
        double total_time = 0.0;
        std::vector<RouteItemStat> items;
    };

} // namespace route

namespace reader {

    enum class QueryType {
        qSTOP,
        qBUS,
        qMAP,
        qROUTE
    };

    struct RouteCommand {
        std::string from;
        std::string to;
    };

    struct StatCommand {
        int id = 0;
        QueryType query_type = QueryType::qSTOP;
        std::variant <std::monostate, std::string, RouteCommand> data;
    };

    struct StatInfo {
        int id = 0;
        QueryType query_type = QueryType::qSTOP;
        std::variant <std::monostate, StopStat, BusStat, route::RouteStat, std::string> info;
    };

} // namespace reader