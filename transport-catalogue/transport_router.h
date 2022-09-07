#pragma once

#include <deque>
#include <map>
#include <stdexcept>

#include "transport_catalogue.h"
#include "graph.h"

namespace catalogue {
    struct RoutingSettings {
        double bus_wait_time = 0.0;
        double bus_velocity = 0.0;
    };

    class TransportRouter {
    private:
        RoutingSettings routing_settings_;
        const TransportCatalogue& cat_;
        graph::DirectedWeightedGraph<BusRouteWeight> route_graph_;

        std::deque<StopPtr> vertex_index_to_stop_;
        std::deque<BusPtr> edge_index_to_bus_;
        std::map<std::string_view, graph::VertexId> stopname_to_vertex_id_;

        template <typename ForwardIt>
        void AddBusStopsEdges(BusPtr bus, ForwardIt first_stop, ForwardIt last_stop);

    public:
        explicit TransportRouter(RoutingSettings settings,
            const TransportCatalogue& cat);

        void SetRoutingSettings(RoutingSettings routing_settings);

        void AddStopVertex(StopPtr);
        void AddBusWaitEdges();
        void AddBusEdges(std::string_view name);

        template <typename Weight>
        const graph::DirectedWeightedGraph<Weight>& GetRouteGraph() const;

        graph::VertexId GetStopVertexIndex(std::string_view stop_name) const;
        BusPtr GetBusByEdgeIndex(graph::EdgeId edge_id) const;
        const graph::Edge<BusRouteWeight>& GetEdgeByIndex(graph::EdgeId edge_id) const;
        StopPtr GetStopByVertexIndex(graph::VertexId vertex_id) const;

        //serialization
        const RoutingSettings& GetRoutingSettings() const;
        const std::deque<StopPtr>& GetVertexIndexToStop() const;
        const std::deque<BusPtr>& GetEdgeIndexToBus() const;

        void SetRouteGraph(graph::DirectedWeightedGraph<BusRouteWeight>&& route_graph);
        void SetVertexIndexToStop(std::deque<StopPtr>&& vertex_index_to_stop);
        void SetEdgeIndexToBus(std::deque<BusPtr>&& edge_index_to_bus);
        void SetStopnameToVertexId(std::map<std::string_view, graph::VertexId>&& stopname_to_vertex_id);

    };

    template <typename Weight>
    const graph::DirectedWeightedGraph<Weight>& TransportRouter::GetRouteGraph() const {
        return route_graph_;
    }

    template <typename ForwardIt>
    void TransportRouter::AddBusStopsEdges(BusPtr bus, ForwardIt first_stop, ForwardIt last_stop) {
        double current_distance = 0.0;
        int span_count = 0;
        for (auto it_from = first_stop; it_from != std::prev(last_stop); ++it_from) {
            for (auto it_to = std::next(it_from); it_to != last_stop; ++it_to) {
                current_distance = current_distance + cat_.GetDistance({ *(std::prev(it_to)), *it_to });
                ++span_count;
                route_graph_.AddEdge({
                    GetStopVertexIndex((*it_from)->name_) + 1,
                    GetStopVertexIndex((*it_to)->name_),
                    {
                        current_distance / routing_settings_.bus_velocity,
                        span_count
                    }
                    });
                edge_index_to_bus_.push_back(bus);
            }
            current_distance = 0.0;
            span_count = 0;
        }
    }

} // namespace catalogue