#pragma once

#include "request_handler.h"
#include "json_builder.h"
#include "map_renderer.h"

namespace json_reader {
    using namespace std::literals;

    struct DistanceSpec {
        int distance_meters;
        std::string dest;
    };

    class JSONReader final {
    public:
        JSONReader(json::Document doc);

        void LoadDataToTC(catalog::TransportCatalogue& catalog) const;
        map_render::RenderSettings GetRenderSettings() const;
        void PrintStats(std::ostream& output, const std::vector<reader::StatInfo>& stats_info);
        std::vector<reader::StatCommand> GetStatCommands() const;
        route::RouteSettings GetRouteSettings() const;

    private:
        json::Document requests_;

        void LoadStopsToTC(catalog::TransportCatalogue& catalog, std::vector<json::Node>::const_iterator first, std::vector<json::Node>::const_iterator last) const;
        void LoadBusToTC(catalog::TransportCatalogue& catalog, const json::Node& node) const;

        Stop ParseStopCommand(const json::Node& node) const;
        std::vector<DistanceSpec> ParseDistances(const json::Node node) const;
        svg::Color ParseColor(const json::Node& node) const;
        json::Node ParseMapStat(const reader::StatInfo& stat_info) const;
        json::Node ParseBusStat(const reader::StatInfo& stat_info) const;
        json::Node ParseStopStat(const reader::StatInfo& stat_info) const;
        json::Node ParseRouteStat(const reader::StatInfo& stat_info) const;

        reader::QueryType DefineRequestType(std::string_view query) const;
    };

}