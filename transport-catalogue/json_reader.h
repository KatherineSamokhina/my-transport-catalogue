#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <utility>
#include <iostream>
#include <tuple>
#include <iomanip>
#include <cassert>
#include <stdexcept>
#include <sstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "request_handler.h"
#include "transport_router.h"
#include "serialization.h"


namespace json_reader {

    struct AddStopRequest {
        std::string name;
        geo::Coordinates cordinates;
        std::unordered_map<std::string, int> distances;
    };

    struct AddBusRequest {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip;
    };

    struct StatRequest {
        int id = 0;
        std::string type;
        std::string name;
    };

    class JsonReader {
    public:
        JsonReader(json::Document document);

        void ReadBaseRequests(json::Document document);
        json::Document ProcessStatRequests(RequestHandler& handler);
        void Fill(catalogue::TransportCatalogue& catalogue, catalogue::TransportRouter& router);

        // ---- rendering ----
        renderer::RenderSettings ReadRenderSettingsFromJSON(const json::Document& document) const;
        renderer::RenderSettings GetRenderSettings() const;

        // ---- routing ----
        catalogue::RoutingSettings ReadRoutingSettings(const json::Document& document) const;
        void SetRoutingSettings(catalogue::RoutingSettings settings, catalogue::TransportCatalogue& catalogue) const;

        // ---- serialization ----
        Serialize::SerializeSettings ReadSerializeSettings(const json::Document& document) const;
    private:
        std::deque<AddStopRequest> add_stop_requests_;
        std::deque<AddBusRequest> add_bus_requests_;

        void AddStopBaseRequest(const json::Node& request);
        void AddBusBaseRequest(const json::Node& request);

        svg::Color ReadColor(const json::Node& node) const;

        json::Node ConvertBusStatToJsonDict(int id, std::optional<BusInfo> bus_stat);
        json::Node ConvertStopInfoToJsonDict(int id, std::optional<StopInfo> bus_stat);
        json::Node ConvertMapToJsonDict(int id, std::string map_as_string);
        json::Node ConvertRouteInfoToJsonDict(int id,
            std::optional<graph::Router<BusRouteWeight>::RouteInfo> route_info,
            RequestHandler& handler);

        void ProcessBusStatRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array);
        void ProcessStopInfoRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array);
        void ProcessMapRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array);
        void ProcessRouteRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array);
        json::Document document_;
    };

} // namespace json_reader