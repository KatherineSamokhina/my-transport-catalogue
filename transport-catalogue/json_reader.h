#pragma once
#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json.h"
#include "domain.h"

class JSONreader final {

public:
	JSONreader() = default;
	explicit JSONreader(catalog::TransportCatalogue& base, MapRenderer& map_renderer, RequestHandler& request)
		: base_(base)
		, request_handler_(request)
		, map_renderer_(map_renderer) {};

	void ReadRequest(const json::Document&);
	void ReadRequests(const json::Document&);
	void ReadTransportCatalogue(std::ostream& ost = std::cout);

private:
	catalog::TransportCatalogue& base_;
	RequestHandler& request_handler_;
	MapRenderer& map_renderer_;
	json::Array base_requests_;
	json::Array stat_requests_;
	json::Array answers_;

	Stop MakeStop(const json::Dict&);
	Bus MakeBus(const json::Dict&);
	Settings MakeRenderSettings(const json::Dict&) const;
	json::Node ReadStop(const json::Dict&);
	json::Node ReadBus(const json::Dict&);
	json::Node ReadMap(const json::Dict& description);

	void LoadStops();
	void LoadBuses();
	void LoadDistances();

	svg::Point SetPoint(const json::Node&)const;
	svg::Color SetColor(const json::Node&)const;
};