#pragma once

#include <string_view>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <optional>

#include "geo.h"
#include "domain.h"

namespace catalog {

	using namespace detail;

	class TransportCatalogue {

	public:
		void AddBus(const Bus&);
		void AddStop(const Stop&);

		[[nodiscard]] std::shared_ptr<Bus> FindBus(const std::string_view bus_number)const;
		[[nodiscard]] std::shared_ptr<Stop> FindStop(const std::string_view stop_name)const;

		std::optional<BusInfo> GetBusInfo(const std::string_view bus)const;
		std::shared_ptr<Buses> GetBusesByStop(const std::string_view) const;

		void AddDistanceBetweenStops(std::shared_ptr<Stop>, std::shared_ptr<Stop>, float);

		float GetDistanceBetweenStops(std::shared_ptr<Stop>, std::shared_ptr<Stop>) const;

		const Buses& GetBuses() const;
		const Stops& GetStops() const;

	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;

		Buses ptr_buses_;
		Stops ptr_stops_;

		std::unordered_map<std::string_view, std::shared_ptr<Bus>> index_buses_;
		std::unordered_map<std::string_view, std::shared_ptr<Stop>> index_stops_;

		std::unordered_map<std::shared_ptr<Bus>, BusInfo> buses_info_;

		std::unordered_map<std::shared_ptr<Stop>, Buses> buses_for_stops_;

		std::unordered_map<std::pair<std::shared_ptr<Stop>, std::shared_ptr<Stop>>, float, DistanceHasher> distance_between_stops_;

		void SetBusStat(std::shared_ptr<Bus> bus);
	};

} //namespace catalog