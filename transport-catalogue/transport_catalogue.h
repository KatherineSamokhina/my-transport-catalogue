#pragma once

#include <string_view>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"

namespace catalog {
    
    using namespace detail;

    class TransportCatalogue {

    public:
        BusPtr AddBus(Bus bus);
        StopPtr AddStop(Stop stop);

        BusPtr FindBus(std::string_view name) const;
        StopPtr FindStop(std::string_view name) const;

        BusStat GetBusInfo(BusPtr bus) const;
        StopStat GetBusesByStop(std::string_view stop_name) const;
            
        void AddDistanceBetweenStops(StopPtr start, const std::pair<std::string_view, int>& end);

        int GetDistanceBetweenStops(StopPtr start, StopPtr end) const;

        void AddStopsToBus(BusPtr bus, std::vector<StopPtr>::const_iterator first, std::vector<StopPtr>::const_iterator last);
        
        std::vector<BusPtr> GetBuses() const;
        std::vector<StopPtr> GetStopsWithRoutes() const;

        const std::unordered_map<std::string_view, StopPtr>& GetNamesToStops() const;
        const std::unordered_map<std::string_view, BusPtr>& GetNamesToBuses() const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        
        std::unordered_map<std::string_view, BusPtr> index_buses_;
        std::unordered_map<std::string_view, StopPtr> index_stops_;
        
        std::unordered_map<StopPtr, std::unordered_set<BusPtr>> buses_for_stops_;
        
        std::unordered_map<std::pair<StopPtr, StopPtr>, int, DistanceHasher> distance_between_stops_;
    };

} // namespace catalog