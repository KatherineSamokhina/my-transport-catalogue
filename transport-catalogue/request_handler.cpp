#include "request_handler.h"

std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    BusInfo stat = db_.GetBusInfo(bus_name);
    if(!stat.IsExsists) {
        return std::nullopt;
    } else {
        return stat;
    }
}

std::optional<StopInfo> RequestHandler::GetStopInfo(const std::string_view& stop_name) const {
    StopInfo info = db_.GetStopInfo(stop_name);
    if(info.IsExsists) {
        return info;
    } else {
        return std::nullopt;
    }
}

RequestHandler::RequestHandler(const TransportCatalogue& db, renderer::MapRenderer& renderer, graph::Router<BusRouteWeight>& router, catalogue::TransportRouter& t_router)
    : db_(db), renderer_(renderer), router_(router), t_router_(t_router)
{
}

std::string RequestHandler::RenderMap() {
    std::deque<BusPtr> buses = db_.GetBusesSorted();
    renderer_.RenderRoutes(buses);

    const auto stops_to_buses = db_.GetStopsToBuses();
    const auto stopname_to_stops = db_.GetStopnameToStops();
    renderer_.RenderStops(stopname_to_stops, stops_to_buses);

    std::ostringstream out;
    renderer_.Render({out, 0, 0});

    return out.str();
}

std::optional<graph::Router<BusRouteWeight>::RouteInfo> RequestHandler::GetRouteInfo(std::string_view stop_from, std::string_view stop_to) const {
    std::optional<graph::Router<BusRouteWeight>::RouteInfo> route_info = router_.BuildRoute(
        t_router_.GetStopVertexIndex(stop_from),
        t_router_.GetStopVertexIndex(stop_to)
    );
    return route_info;
}

const graph::Edge<BusRouteWeight>& RequestHandler::GetEdgeByIndex(graph::EdgeId edge_id) const {
    return t_router_.GetEdgeByIndex(edge_id);
}

BusPtr RequestHandler::GetBusByEdgeIndex(graph::EdgeId edge_id) const {
    return t_router_.GetBusByEdgeIndex(edge_id);
}

StopPtr RequestHandler::GetStopByVertexIndex(graph::VertexId vertex_id) const {
    return t_router_.GetStopByVertexIndex(vertex_id);
}

