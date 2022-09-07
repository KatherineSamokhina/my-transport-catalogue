#include "transport_catalogue.h"

namespace catalogue {
    void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& stops, BusType type) {
        std::vector<StopPtr> stops_ptr;
        auto it = buses_.insert(buses_.end(), std::move(Bus{ std::string(name), stops_ptr, type, bus_count_++ }));

        for_each(stops.begin(), stops.end(), [&stops_ptr, &it, this](std::string_view stop_name) {
            if (StopPtr stop_ptr = FindStop(stop_name)) {
                it->stops_.push_back(std::move(stop_ptr));
                buses_for_stops_[stop_ptr].insert(&*it);
            }

            });
        index_buses_[std::string_view{ it->name_ }] = &(*it);
    }

    void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates) {
        auto it = stops_.emplace(stops_.end(), std::move(Stop{ std::string(name), coordinates, stop_count_++ }));
        index_stops_[std::string_view{ it->name_ }] = &(*it);
    }

    StopPtr TransportCatalogue::FindStop(std::string_view name) const {
        if (index_stops_.count(name) == 0) {
            return nullptr;
        }
        return index_stops_.at(name);
    }

    BusPtr TransportCatalogue::FindBus(std::string_view name) const {
        if (index_buses_.count(name) == 0) {
            return nullptr;
        }
        return index_buses_.at(name);
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
        if (busname_to_businfo_.count(name) == 0) {
            return ComputeBusInfo(name);
        }
        return *(busname_to_businfo_.at(name));
    }

    StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        if (index_stops_.count(stop_name) != 0) {
            StopPtr stop_ptr = FindStop(stop_name);
            if (buses_for_stops_.count(stop_ptr) != 0) {
                StopInfo stop_info{ {}, true };

                std::for_each(buses_for_stops_.at(stop_ptr).begin(),
                    buses_for_stops_.at(stop_ptr).end(),
                    [&](auto bus) {
                        stop_info.buses_.insert(bus->name_);
                    });
                return stop_info;
            }
            else {
                return StopInfo{ {}, true };
            }
        }
        else {
            return StopInfo{ {}, false };
        }
    }

    void TransportCatalogue::SetDistance(std::pair<StopPtr, StopPtr> p, uint64_t distance) {
        distance_between_stops_[p] = distance;

        if (distance_between_stops_.count({ p.second, p.first }) == 0 || p.first == p.second) {
            distance_between_stops_[{p.second, p.first}] = distance;
        }
    }

    uint64_t TransportCatalogue::GetDistance(std::pair<StopPtr, StopPtr> p) const {
        if (distance_between_stops_.count(p)) {
            return distance_between_stops_.at(p);
        }
        else {
            return 0u;
        }
    }

    BusInfo TransportCatalogue::ComputeBusInfo(std::string_view name) const {
        BusPtr bus{ nullptr };
        size_t stops_count = 0, unique_stops_count = 0;
        double length_geo = 0;
        uint64_t length_road = 0;
        double curvature = 0.0;
        if (bus = FindBus(name), bus) {
            stops_count = bus->stops_.size();

            std::unordered_set<std::string_view, std::hash<std::string_view>, std::equal_to<std::string_view>> unique_stops;
            for_each(bus->stops_.begin(), bus->stops_.end(), [&unique_stops](StopPtr stop) {
                unique_stops.insert(std::string_view(stop->name_));
                });
            unique_stops_count = unique_stops.size();

            length_geo = std::transform_reduce(
                next(bus->stops_.begin()), bus->stops_.end(),
                bus->stops_.begin(),
                0.0,
                std::plus<>(),
                [](StopPtr s, StopPtr prev_s) {
                    if (s == prev_s) { return 0.0; }
                    return ComputeDistance(prev_s->cordinates_, s->cordinates_);
                }
            );

            length_road = std::transform_reduce(
                next(bus->stops_.begin()), bus->stops_.end(),
                bus->stops_.begin(),
                0ULL,
                std::plus<>(),
                [this, &bus](StopPtr s, StopPtr prev_s) {
                    if (bus->bus_type_ == BusType::CYCLED) {
                        return GetDistance({ prev_s, s });
                    }
                    else if (bus->bus_type_ == BusType::ORDINARY) {
                        return GetDistance({ prev_s, s }) + GetDistance({ s, prev_s });
                    }
                    else {
                        uint64_t c = 0;
                        return c;
                    }
                }
            );
            if (bus->bus_type_ == BusType::ORDINARY) {
                length_geo *= 2;
                stops_count = stops_count * 2 - 1;
            }
            curvature = static_cast<long double>(length_road) / static_cast<long double>(length_geo);
            return BusInfo{ stops_count, unique_stops_count, length_road, curvature, true };
        }
        else {
            return BusInfo{};
        }
    }

    std::deque<BusPtr> TransportCatalogue::GetBusesSorted() const {
        std::deque<BusPtr> result;
        for (const auto& [busname, bus] : index_buses_) {
            result.push_back(bus);
        }
        return result;
    }

    const std::unordered_map<StopPtr, std::set<BusPtr>>& TransportCatalogue::GetStopsToBuses() const {
        return buses_for_stops_;
    }

    const std::map<std::string_view, StopPtr>& TransportCatalogue::GetStopnameToStops() const {
        return index_stops_;
    }

    const std::deque<Stop>& TransportCatalogue::GetStops() const {
        return stops_;
    }

    const std::deque<Bus>& TransportCatalogue::GetBuses() const {
        return buses_;
    }

    const std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher>&
        TransportCatalogue::GetIntervalsToDistance() const {
        return distance_between_stops_;
    }

    void TransportCatalogue::SetStops(std::deque<Stop>&& stops) {
        stops_.swap(stops);
    }

    void TransportCatalogue::SetStopnameToStop(std::map<std::string_view, StopPtr>&& stopname_to_stop) {
        index_stops_ = stopname_to_stop;
    }


    void TransportCatalogue::SetBuses(std::deque<Bus>&& buses) {
        buses_.swap(buses);
    }
    void TransportCatalogue::SetBusnameToBus(std::map<std::string_view, BusPtr>&& busname_to_bus) {
        index_buses_ = busname_to_bus;
    }
    void TransportCatalogue::SetStopsToBuses(std::unordered_map<StopPtr, std::set<BusPtr>>&& stops_to_buses) {
        buses_for_stops_ = stops_to_buses;
    }
    void TransportCatalogue::SetDistanceBetweenStops(std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher>&& intervals_to_distance) {
        distance_between_stops_ = intervals_to_distance;
    }
} // namespace catalogue


