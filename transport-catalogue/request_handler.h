#pragma once
#include <optional>
#include <memory>
#include <set>

#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:

	RequestHandler(const catalog::TransportCatalogue&, const MapRenderer&);

	std::optional<BusInfo> GetBusInfo(const std::string_view) const;
	std::shared_ptr<Buses> GetBusesByStop(const std::string_view) const;

	void RenderMap(std::ostream& out = std::cout)const;

private:
	const catalog::TransportCatalogue& base_;
	const MapRenderer& map_renderer_;
};