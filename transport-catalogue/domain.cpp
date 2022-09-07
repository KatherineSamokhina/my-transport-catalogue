#include "domain.h"

bool BusInfo::IsEmpty() const {
    return stops_count == 0 && unique_stops_count == 0 && route_length < MIN&& curvature == 0;
}

size_t CoordinatesHasher::operator()(geo::Coordinates c) const {
	std::hash<double> ptr_hasher{};
	return 17 * ptr_hasher(c.lat) + ptr_hasher(c.lng);
}

bool BusRouteWeight::operator<(const BusRouteWeight& other) const {
    return time < other.time;
}

bool BusRouteWeight::operator>(const BusRouteWeight& other) const {
    return time > other.time;
}

bool BusRouteWeight::operator==(const BusRouteWeight& other) const {
    return time == other.time;
}

bool BusRouteWeight::operator!=(const BusRouteWeight& other) const {
    return time != other.time;
}

BusRouteWeight BusRouteWeight::operator+(const BusRouteWeight& other) const {
    return { time + other.time, span + other.span };
}