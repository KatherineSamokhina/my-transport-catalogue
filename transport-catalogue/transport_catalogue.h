#pragma once

#include <string_view>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <numeric>
#include <algorithm>

#include "domain.h"

namespace catalogue
{
    class TransportCatalogue {

    public:

        void AddBus(std::string_view name, const std::vector<std::string>& stops, BusType type);
        void AddStop(std::string_view name, geo::Coordinates coordinates);

        BusPtr FindBus(std::string_view name) const;
        StopPtr FindStop(std::string_view name) const;
        
        BusInfo GetBusInfo(std::string_view name) const;
        StopInfo GetStopInfo(std::string_view stop_name) const;

        void SetDistance(std::pair<StopPtr, StopPtr> p, uint64_t distance);
        uint64_t GetDistance(std::pair<StopPtr, StopPtr> p) const;

        const std::unordered_map<StopPtr, std::set<BusPtr>>& GetStopsToBuses() const;
        const std::map<std::string_view, StopPtr>& GetStopnameToStops() const;

        std::deque<BusPtr> GetBusesSorted() const;

        // serialization
        const std::deque<Stop>& GetStops() const;
        const std::deque<Bus>& GetBuses() const;
        const std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher>& GetIntervalsToDistance() const;

        void SetStops(std::deque<Stop>&& stops);
        void SetStopnameToStop(std::map<std::string_view, StopPtr>&& stopname_to_stop);
        void SetBuses(std::deque<Bus>&& buses);
        void SetBusnameToBus(std::map<std::string_view, BusPtr>&& busname_to_bus);
        void SetStopsToBuses(std::unordered_map<StopPtr, std::set<BusPtr>>&& stops_to_buses);
        void SetDistanceBetweenStops(std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher>&& intervals_to_distance);

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        std::map<std::string_view, BusPtr> index_buses_;
        std::map<std::string_view, StopPtr> index_stops_;
        
        std::deque<BusInfo> bus_infos_;
        std::unordered_map<std::string_view, const BusInfo*> busname_to_businfo_;
        BusInfo ComputeBusInfo(std::string_view name) const;

        std::unordered_map<StopPtr, std::set<BusPtr>> buses_for_stops_;

        std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher> distance_between_stops_;

        friend class TransportRouter;
        
        int stop_count_ = 0;
        int bus_count_ = 0;
    };
} // namespace catalogue

