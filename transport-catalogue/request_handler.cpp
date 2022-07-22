#include "request_handler.h"

RequestHandler::RequestHandler(const catalog::TransportCatalogue& base, const MapRenderer& map_renderer) 
	: base_(base)
	, map_renderer_(map_renderer) 
{}

std::optional<BusInfo> RequestHandler::GetBusInfo(const std::string_view number) const {
	return base_.GetBusInfo(number);
}

std::shared_ptr<Buses> RequestHandler::GetBusesByStop(const std::string_view stop_name) const {
	return base_.GetBusesByStop(stop_name);
}

void RequestHandler::RenderMap(std::ostream& out) const {
	map_renderer_.GetDocument().Render(out);
}