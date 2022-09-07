#pragma once

#include <string>
#include <vector>
#include <set>

#include "geo.h"

static const double MIN = 1e-6;

struct Stop {
    std::string name_;
    geo::Coordinates cordinates_;
    int id = 0;
};
using StopPtr = const Stop*;

enum class BusType {
    ORDINARY,
    CYCLED
};
struct Bus {
    std::string name_;
    std::vector<StopPtr> stops_;
    BusType bus_type_;
    int id = 0;
};
using BusPtr = const Bus*;

struct BusInfo {
    size_t stops_count = 0u;
    size_t unique_stops_count = 0u;
    uint64_t route_length = 0;
    double curvature = 0.0;

    bool IsEmpty() const;
    bool IsExsists = false;
};
struct StopInfo {
    std::set<std::string_view> buses_;
    bool IsExsists = false;
};

struct DistanceHasher {
    template <class T>
    std::size_t operator()(const std::pair<T, T>& pair) const {
        return std::hash<T>()(pair.first) + 37 * std::hash<T>()(pair.second);
    }
};

struct CoordinatesHasher {
    size_t operator()(geo::Coordinates c) const;
};

struct BusRouteWeight {
    double time = 0.0;
    int span = 0;

    bool operator<(const BusRouteWeight& other) const;
    bool operator>(const BusRouteWeight& other) const;
    bool operator==(const BusRouteWeight& other) const;
    bool operator!=(const BusRouteWeight& other) const;

    BusRouteWeight operator+(const BusRouteWeight& other) const;
};