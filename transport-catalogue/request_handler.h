#pragma once

#include <optional>
#include <sstream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "svg.h"
#include "router.h"

using catalogue::TransportCatalogue;

class RequestHandler {
public:

    RequestHandler(const TransportCatalogue& db, 
    renderer::MapRenderer& renderer, 
    graph::Router<BusRouteWeight>& router,
    catalogue::TransportRouter& t_router);

    std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

    std::string RenderMap();

    std::optional<StopInfo> GetStopInfo(const std::string_view& bus_name) const;

    std::optional<graph::Router<BusRouteWeight>::RouteInfo> GetRouteInfo(std::string_view stop_from, std::string_view stop_to) const;


    const graph::Edge<BusRouteWeight>& GetEdgeByIndex(graph::EdgeId edge_id) const;
    BusPtr GetBusByEdgeIndex(graph::EdgeId edge_id) const;
    StopPtr GetStopByVertexIndex(graph::VertexId vertex_id) const;

private:
    const TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

    const graph::Router<BusRouteWeight>& router_;
    const catalogue::TransportRouter& t_router_;
};



