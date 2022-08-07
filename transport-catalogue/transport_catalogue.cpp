#include "transport_catalogue.h"

namespace catalog {
    using namespace detail;

    BusPtr TransportCatalogue::AddBus(Bus bus) {
        const auto& ref = buses_.emplace_back(std::move(bus));
        index_buses_[ref.name] = &ref;
        return &ref;
    }
    StopPtr TransportCatalogue::AddStop(Stop stop) {
        const auto& ref = stops_.emplace_back(std::move(stop));
        index_stops_[ref.name] = &ref;
        buses_for_stops_[&ref];
        return &ref;
    }

    BusPtr TransportCatalogue::FindBus(std::string_view name) const {
        auto it = index_buses_.find(name);

        if (it != index_buses_.end()) {
            return it->second;
        }

        return nullptr;
    }
    StopPtr TransportCatalogue::FindStop(std::string_view name) const {
        auto it = index_stops_.find(name);

        if (it != index_stops_.end()) {
            return it->second;
        }
        return nullptr;
    }

    BusStat TransportCatalogue::GetBusInfo(BusPtr bus) const {
        BusStat result;

        StopPtr last_stop = bus->stops[0];
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            StopPtr stop = bus->stops[i];
            result.curvature += geo::ComputeDistance(last_stop->position, stop->position);
            result.route_length += GetDistanceBetweenStops(last_stop, stop);
            last_stop = stop;
        }

        if (!bus->is_roundtrip) {
            for (size_t i = bus->stops.size() - 1; i-- > 0u;) {
                StopPtr stop = bus->stops[i];
                result.curvature += geo::ComputeDistance(last_stop->position, stop->position);
                result.route_length += GetDistanceBetweenStops(last_stop, stop);
                last_stop = stop;
            }
            result.stops_count = bus->stops.size() * 2 - 1;
        }
        else {
            result.stops_count = bus->stops.size();
        }

        result.unique_stops = std::unordered_set<StopPtr>(bus->stops.cbegin(), bus->stops.cend()).size();
        result.curvature = static_cast<double>(result.route_length) / result.curvature;

        return result;
    }
    StopStat TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        StopPtr stop = FindStop(stop_name);

        auto it = buses_for_stops_.find(stop);
        std::set<std::string_view> result;
        bool is_found = false;
        if (it != buses_for_stops_.end()) {
            for (const auto& stop : it->second) {
                result.insert(stop->name);
            }
            is_found = true;
        }

        return { is_found, result };
    }

    void TransportCatalogue::AddDistanceBetweenStops(StopPtr start, const std::pair<std::string_view, int>& end) {
        StopPtr end_stop = FindStop(end.first);
        distance_between_stops_[{ start, end_stop }] = end.second;
    }

    int TransportCatalogue::GetDistanceBetweenStops(StopPtr start, StopPtr end) const {
        auto it = distance_between_stops_.find({ start, end });

        if (it != distance_between_stops_.end()) {
            return it->second;
        }

        it = distance_between_stops_.find({ end, start });

        if (it != distance_between_stops_.end()) {
            return it->second;
        }

        return 0;
    }
    
    void TransportCatalogue::AddStopsToBus(BusPtr bus,
        std::vector<StopPtr>::const_iterator first, std::vector<StopPtr>::const_iterator last) {
        for (auto it = first; it != last; ++it) {
            buses_for_stops_[*it].insert(bus);
        }
    }

    std::vector<BusPtr> TransportCatalogue::GetBuses() const {
        std::vector<BusPtr> result;
        for (const auto& [_, value] : index_buses_) {
            result.push_back(value);
        }
        return result;
    }

    std::vector<StopPtr> TransportCatalogue::GetStopsWithRoutes() const {
        std::vector<StopPtr> result;
        for (const auto& [key, value] : buses_for_stops_) {
            if (!value.empty())
                result.push_back(key);
        }

        return result;
    }

    const std::unordered_map<std::string_view, StopPtr>& TransportCatalogue::GetNamesToStops() const {
        return index_stops_;
    }

    const std::unordered_map<std::string_view, BusPtr>& TransportCatalogue::GetNamesToBuses() const {
        return index_buses_;
    }

} //namespace catalog