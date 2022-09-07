#include "serialization.h"

namespace Serialize
{
    catalogue::TransportCatalogue Deserializer::GetTransportCatalogue() const {
        catalogue::TransportCatalogue result;

        const tc_pb::TransportCatalogue& pb_catalogue = pb_base_.cat();

        {
            std::deque<Stop> stops;
            std::map<std::string_view, StopPtr> stopname_to_stop;
            for (const auto& pb_stop : pb_catalogue.stops()) {
                Stop& emplaced = stops.emplace_back(
                    Stop{
                        std::string(pb_stop.name()),
                        geo::Coordinates{
                            pb_stop.coordinates().lat(),
                            pb_stop.coordinates().lng()
                        },
                        pb_stop.id()
                    }
                );

                stopname_to_stop[std::string_view(emplaced.name_)] = &emplaced;
            }

            result.SetStops(std::move(stops));
            result.SetStopnameToStop(std::move(stopname_to_stop));
        }
        {
            std::deque<Bus> buses;
            std::map<std::string_view, BusPtr> busname_to_bus;
            for (const auto& pb_bus : pb_catalogue.buses()) {
                std::vector<StopPtr> stops;
                for (int stop_id : pb_bus.stops()) {
                    assert(result.GetStops().at(stop_id).id == stop_id);
                    stops.push_back(&(result.GetStops().at(stop_id)));
                }

                BusType bus_type;
                if (pb_bus.bus_type_cycled()) {
                    bus_type = BusType::CYCLED;
                }
                else {
                    bus_type = BusType::ORDINARY;
                }
                Bus current_bus{
                    std::string(pb_bus.name()),
                    std::move(stops),
                    bus_type,
                    pb_bus.id()
                };
                Bus& emlplaced = buses.emplace_back(std::move(current_bus));

                busname_to_bus[std::string_view(emlplaced.name_)] = &(emlplaced);
            }

            result.SetBuses(std::move(buses));
            result.SetBusnameToBus(std::move(busname_to_bus));
        }
        {
            std::unordered_map<StopPtr, std::set<BusPtr>> stops_to_buses;

            for (const auto& stop_to_buses : pb_catalogue.stops_to_buses()) {
                int stop_id = stop_to_buses.stop_id();
                StopPtr stop_ptr = &(result.GetStops().at(stop_id));
                assert(stop_id == stop_ptr->id);

                std::set<BusPtr> current_stop_buses;
                for (int bus_id : stop_to_buses.bus_id()) {
                    BusPtr bus_ptr = &(result.GetBuses().at(bus_id));
                    assert(bus_id == bus_ptr->id);
                    stops_to_buses[stop_ptr].insert(bus_ptr);
                }
            }

            result.SetStopsToBuses(std::move(stops_to_buses));
        }
        {
            std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, DistanceHasher> intervals_to_distance;

            for (const auto& interval : pb_catalogue.intervals_to_distance()) {
                int from_id = interval.from_id();
                int to_id = interval.to_id();
                int64_t distance = interval.distance();

                StopPtr stop_ptr_from = &(result.GetStops().at(from_id));

                StopPtr stop_ptr_to = &(result.GetStops().at(to_id));

                intervals_to_distance[{stop_ptr_from, stop_ptr_to}] = distance;
            }

            result.SetDistanceBetweenStops(std::move(intervals_to_distance));
        }
        return result;
    }

    catalogue::RoutingSettings Deserializer::GetRoutingSettings() const {
        catalogue::RoutingSettings result;

        result.bus_velocity = pb_base_.routing_settings().bus_velocity();
        result.bus_wait_time = pb_base_.routing_settings().bus_wait_time();

        return result;
    }

    renderer::RenderSettings Deserializer::GetRenderSettings() const {
        renderer::RenderSettings result;

        const tc_pb::RenderSettings& pb_render_settings = pb_base_.rendder_settings();

        result.width = pb_render_settings.width();
        result.height = pb_render_settings.height();
        result.padding = pb_render_settings.padding();
        result.line_width = pb_render_settings.line_width();
        result.stop_radius = pb_render_settings.stop_radius();

        result.bus_label_font_size = pb_render_settings.bus_label_font_size();
        result.bus_label_offset = {
            pb_render_settings.bus_label_offset().x(),
            pb_render_settings.bus_label_offset().y(),
        };

        result.stop_label_font_size = pb_render_settings.stop_label_font_size();
        result.stop_label_offset = {
            pb_render_settings.stop_label_offset().x(),
            pb_render_settings.stop_label_offset().y(),
        };

        result.underlayer_color = ExtractSVGColorFromPBColor(pb_render_settings.underlayer_color());
        result.underlayer_width = pb_render_settings.underlayer_width();

        for (const auto& pb_color : pb_render_settings.color_palette()) {
            result.color_palette.push_back(ExtractSVGColorFromPBColor(pb_color));
        }

        return result;
    }

    svg::Color Deserializer::ExtractSVGColorFromPBColor(tc_pb::Color pb_color) {
        svg::Color result;

        if (pb_color.is_initialized()) {
            if (pb_color.has_rgb_color()) {

                result = svg::Rgb{
                    static_cast<uint8_t>(pb_color.rgb_color().red()),
                    static_cast<uint8_t>(pb_color.rgb_color().green()),
                    static_cast<uint8_t>(pb_color.rgb_color().blue())
                };
            }
            else if (pb_color.has_rgba_color()) {

                result = svg::Rgba{
                    static_cast<uint8_t>(pb_color.rgba_color().red()),
                    static_cast<uint8_t>(pb_color.rgba_color().green()),
                    static_cast<uint8_t>(pb_color.rgba_color().blue()),
                    pb_color.rgba_color().opacity()
                };
            }
            else {
                result = pb_color.string_color();
            }
        }

        return result;
    }

    catalogue::TransportRouter Deserializer::GetTransportRouter(const catalogue::TransportCatalogue& catalogue) const {
        catalogue::TransportRouter result(GetRoutingSettings(), catalogue);

        std::deque<StopPtr> vertex_index_to_stop;
        std::map<std::string_view, graph::VertexId> stopname_to_vertex_id;

        for (const int32_t stop_id : pb_base_.transport_router().vertex_index_to_stop()) {
            StopPtr& emplaced = vertex_index_to_stop.emplace_back(&catalogue.GetStops().at(stop_id));
            stopname_to_vertex_id[emplaced->name_] = 2 * stop_id;
        }

        std::deque<BusPtr> edge_index_to_bus;
        for (auto bus_id : pb_base_.transport_router().edge_index_to_bus()) {
            if (bus_id.isinitialized()) {
                edge_index_to_bus.emplace_back(&catalogue.GetBuses().at(bus_id.bus_id()));
            }
            else {
                edge_index_to_bus.emplace_back(nullptr);
            }
        }

        result.SetVertexIndexToStop(std::move(vertex_index_to_stop));
        result.SetStopnameToVertexId(std::move(stopname_to_vertex_id));
        result.SetEdgeIndexToBus(std::move(edge_index_to_bus));

        graph::DirectedWeightedGraph<BusRouteWeight> route_graph(vertex_index_to_stop.size());
        for (const auto& pb_edge : pb_base_.transport_router().route_graph().edges()) {

            route_graph.AddEdge(
                graph::Edge<BusRouteWeight>{
                pb_edge.vertex_id_from(),
                    pb_edge.vertex_id_to(),
                    BusRouteWeight{
                        pb_edge.weight().time(),
                        pb_edge.weight().span()
                }
            }
            );
        }
        result.SetRouteGraph(std::move(route_graph));

        return result;
    }

    graph::Router<BusRouteWeight> Deserializer::GetRouter(const graph::DirectedWeightedGraph<BusRouteWeight>& graph) const {

        graph::Router<BusRouteWeight>::RoutesInternalData routes_internal_data;
        routes_internal_data.resize(graph.GetVertexCount());

        const tc_pb::Router& pb_router = pb_base_.router();
        size_t from_index = 0;
        for (const tc_pb::RouteInternalDataRow pb_route_internal_data_row : pb_router.routes_internal_data()) {
            routes_internal_data[from_index].resize(graph.GetVertexCount());
            size_t to_index = 0;
            for (const tc_pb::RouteInternalData& pb_route_internal_data : pb_route_internal_data_row.route_internal_data_row()) {
                if (pb_route_internal_data.has_weight()) {
                    routes_internal_data[from_index][to_index] = {
                        {pb_route_internal_data.weight().time(), pb_route_internal_data.weight().span()},
                        std::nullopt
                    };
                    if (pb_route_internal_data.has_prev_edge()) {
                        routes_internal_data[from_index][to_index].value().prev_edge = pb_route_internal_data.prev_edge().prev_edge_id();
                    }
                }

                ++to_index;
            }

            ++from_index;
        }

        graph::Router<BusRouteWeight> result(graph, std::move(routes_internal_data));

        return result;
    }

} // namespace Serialize


