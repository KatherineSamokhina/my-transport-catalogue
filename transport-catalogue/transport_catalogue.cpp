#include <algorithm>
#include <numeric>
#include <optional>

#include "transport_catalogue.h"

namespace catalog {
	using namespace detail;

	void TransportCatalogue::AddStop(const Stop& stop) {
		const auto& iter_stop = stops_.emplace(stops_.end(), stop);
		const auto ptr_stop = std::make_shared<Stop>(*iter_stop);
		ptr_stops_.emplace(ptr_stop);
		buses_for_stops_[ptr_stop];
		index_stops_[ptr_stop->name] = ptr_stop;
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		const auto& iter_bus = buses_.emplace(buses_.end(), bus);
		const auto ptr_bus = std::make_shared<Bus>(*iter_bus);
		ptr_buses_.emplace(ptr_bus);
		for (auto& stop : ptr_bus->stops) {
			stop->free = false;
			buses_for_stops_[stop].emplace(ptr_bus);
		}
		SetBusStat(ptr_bus);
		index_buses_[ptr_bus->number] = ptr_bus;

	}

	[[nodiscard]] std::shared_ptr<Bus> TransportCatalogue::FindBus(const std::string_view bus) const {
		if (index_buses_.count(bus) != 0) {
			return index_buses_.at(bus);
		}
		return nullptr;
	}

	[[nodiscard]] std::shared_ptr<Stop> TransportCatalogue::FindStop(const std::string_view stop) const {
		if (index_stops_.count(stop) != 0) {
			return index_stops_.at(stop);
		}
		return nullptr;
	}

	std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string_view bus_number)const {
		const std::shared_ptr<Bus>& bus = FindBus(bus_number);
		if (!bus) {
			return std::nullopt;
		}
		return buses_info_.at(bus);
	}

	std::shared_ptr<Buses> TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
		std::shared_ptr<Stop> stop = FindStop(stop_name);
		if (!stop) {
			return nullptr;
		}
		auto test = buses_for_stops_.at(stop);
		return std::make_shared<Buses>(buses_for_stops_.at(stop));
	}

	void TransportCatalogue::AddDistanceBetweenStops(std::shared_ptr<Stop> from, std::shared_ptr<Stop> to, float distance) {
		distance_between_stops_[std::make_pair(from, to)] = distance;
	}

	float TransportCatalogue::GetDistanceBetweenStops(std::shared_ptr<Stop> stop1, std::shared_ptr<Stop> stop2) const {

		auto pair = std::make_pair(stop1, stop2);
		if (distance_between_stops_.count(pair) != 0) {
			return distance_between_stops_.at(pair);
		}
		pair = std::make_pair(stop1, stop2);
		if (distance_between_stops_.count(pair) != 0) {
			return distance_between_stops_.at(pair);
		}
		return 0.0;
	}

	void TransportCatalogue::SetBusStat(std::shared_ptr<Bus> bus) {

		std::unordered_set<std::string> unique_stops;
		BusInfo info;
		const std::vector<std::shared_ptr<Stop>>& v_stops = bus->stops;

		std::for_each(v_stops.begin(), v_stops.end(), [&unique_stops](auto stop) {
			unique_stops.insert(stop->name);
			});

		double geographical_length = std::transform_reduce(bus->stops.begin() + 1, bus->stops.end(),
			bus->stops.begin(), 0.0, std::plus<>{}, [](const auto s2, const auto s1) {
				return ComputeDistance(s1->coord, s2->coord);
			}
		);

		info.stops = bus->stops.size();
		info.unique_stops = unique_stops.size();

		info.route_length = std::transform_reduce(bus->stops.begin() + 1, bus->stops.end(),
			bus->stops.begin(), 0, std::plus<>{}, [this](const auto s2, const auto s1) {
				return GetDistanceBetweenStops(s1, s2);
			}
		);

		if (!bus->is_ring) {
			geographical_length *= 2;

			info.stops = info.stops * 2 - 1;

			info.route_length += std::transform_reduce(bus->stops.rbegin() + 1, bus->stops.rend(),
				bus->stops.rbegin(), 0, std::plus<>{}, [this](const auto s2, const auto s1) {
					return GetDistanceBetweenStops(s1, s2);
				}
			);
		}

		info.curvature = info.route_length / geographical_length;

		buses_info_[bus] = std::move(info);
	}

	const Buses& TransportCatalogue::GetBuses() const {
		return ptr_buses_;
	}

	const Stops& TransportCatalogue::GetStops() const {
		return ptr_stops_;
	}

} //namespace catalog