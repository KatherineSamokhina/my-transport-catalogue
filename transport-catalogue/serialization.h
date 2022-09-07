#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include "router.h"

namespace Serialize {
    struct SerializeSettings {
        std::string file;
    };

    class Serializer {

    public:
        Serializer() = delete;
        Serializer(const catalogue::TransportCatalogue& catalogue,
            const catalogue::TransportRouter& transport_router,
            const renderer::RenderSettings& render_settings,
            const SerializeSettings serialization_settings,
            const graph::Router<BusRouteWeight>& router)
            : catalogue_(catalogue)
            , routing_settings_(transport_router.GetRoutingSettings())
            , render_settings_(render_settings)
            , serialize_settings_(serialization_settings)
            , transport_router_(transport_router)
            , router_(router)
        {
            tc_pb::TransportCatalogue pb_catalogue_;
            
            for (const auto& stop : catalogue_.GetStops()) {
                tc_pb::Stop pb_stop;
                pb_stop.set_id(stop.id);
                pb_stop.set_name(stop.name_);
                pb_stop.mutable_coordinates()->set_lat(stop.cordinates_.lat);
                pb_stop.mutable_coordinates()->set_lng(stop.cordinates_.lng);
                pb_catalogue_.mutable_stops()->Add(std::move(pb_stop));
            }

            for (const auto& bus : catalogue_.GetBuses()) {
                tc_pb::Bus pb_bus;
                pb_bus.set_id(bus.id);
                pb_bus.set_name(bus.name_);

                for (const auto& stop : bus.stops_) {
                    pb_bus.add_stops(stop->id);
                }

                switch (bus.bus_type_)
                {
                case BusType::ORDINARY:
                    pb_bus.set_bus_type_cycled(false);
                    break;
                case BusType::CYCLED:
                    pb_bus.set_bus_type_cycled(true);
                    break;
                default:
                    break;
                }
                pb_catalogue_.mutable_buses()->Add(std::move(pb_bus));
            }

            for (const auto& stop_to_buses : catalogue_.GetStopsToBuses()) {
                tc_pb::StopToBuses pb_stop_to_buses;

                int stop_id = stop_to_buses.first->id;
                pb_stop_to_buses.set_stop_id(stop_id);

                const std::set<BusPtr>& buses = stop_to_buses.second;
                for (const auto& bus : buses) {
                    pb_stop_to_buses.mutable_bus_id()->Add(bus->id);
                }
                pb_catalogue_.mutable_stops_to_buses()->Add(std::move(pb_stop_to_buses));
            }

            for (const auto& interval_to_distance : catalogue_.GetIntervalsToDistance()) {
                tc_pb::IntervalToDistance pb_interval_to_distance;
                int from_id = interval_to_distance.first.first->id;
                int to_id = interval_to_distance.first.second->id;
                int64_t distance = interval_to_distance.second;
                pb_interval_to_distance.set_from_id(from_id);
                pb_interval_to_distance.set_to_id(to_id);
                pb_interval_to_distance.set_distance(distance);
                pb_catalogue_.mutable_intervals_to_distance()->Add(std::move(pb_interval_to_distance));
            }

            *pb_base_.mutable_cat() = std::move(pb_catalogue_);

            FillRoutingSettings();

            FillRenderSettings();

            FillTransportRouter();

            FillRouter();
        }

        void SaveTo(const std::filesystem::path& path) const {
            std::ofstream out(path, std::ios::binary);

            pb_base_.SerializeToOstream(&out);
        }

        void Save() const {
            std::ofstream out(std::filesystem::path(serialize_settings_.file), std::ios::binary);
            pb_base_.SerializeToOstream(&out);
        }

    private:
        const catalogue::TransportCatalogue& catalogue_;
        const catalogue::RoutingSettings& routing_settings_;
        const renderer::RenderSettings& render_settings_;
        const SerializeSettings serialize_settings_;
        const catalogue::TransportRouter& transport_router_;
        const graph::Router<BusRouteWeight>& router_;
        tc_pb::TransportBase pb_base_;


        struct RenderSettingsColorVisitor {
            tc_pb::Color& pb_color;

            void operator()(std::monostate) {
                pb_color.set_is_initialized(false);
            }
            void operator()(const std::string& color) {
                *pb_color.mutable_string_color() = color;
                pb_color.set_is_initialized(true);
            }
            void operator()(svg::Rgb rgb) {
                pb_color.mutable_rgb_color()->set_red(rgb.red);
                pb_color.mutable_rgb_color()->set_green(rgb.green);
                pb_color.mutable_rgb_color()->set_blue(rgb.blue);
                pb_color.set_is_initialized(true);
            }
            void operator()(svg::Rgba rgba) {
                pb_color.mutable_rgba_color()->set_red(rgba.red);
                pb_color.mutable_rgba_color()->set_green(rgba.green);
                pb_color.mutable_rgba_color()->set_blue(rgba.blue);
                pb_color.mutable_rgba_color()->set_opacity(rgba.opacity);
                pb_color.set_is_initialized(true);
            }
        };

        void FillRoutingSettings() {
            tc_pb::RoutingSettings pb_routing_settings_;

            pb_routing_settings_.set_bus_velocity(routing_settings_.bus_velocity);
            pb_routing_settings_.set_bus_wait_time(routing_settings_.bus_wait_time);

            *pb_base_.mutable_routing_settings() = std::move(pb_routing_settings_);
        }

        void FillRenderSettings() {
            tc_pb::RenderSettings pb_render_settings;
            pb_render_settings.set_width(render_settings_.width);
            pb_render_settings.set_height(render_settings_.height);
            pb_render_settings.set_padding(render_settings_.padding);
            pb_render_settings.set_line_width(render_settings_.line_width);
            pb_render_settings.set_stop_radius(render_settings_.stop_radius);

            pb_render_settings.set_bus_label_font_size(render_settings_.bus_label_font_size);
            pb_render_settings.mutable_bus_label_offset()->set_x(render_settings_.bus_label_offset.x);
            pb_render_settings.mutable_bus_label_offset()->set_y(render_settings_.bus_label_offset.y);

            pb_render_settings.set_stop_label_font_size(render_settings_.stop_label_font_size);
            pb_render_settings.mutable_stop_label_offset()->set_x(render_settings_.stop_label_offset.x);
            pb_render_settings.mutable_stop_label_offset()->set_y(render_settings_.stop_label_offset.y);

            tc_pb::Color pb_color;
            visit(RenderSettingsColorVisitor{ pb_color }, render_settings_.underlayer_color);
            *pb_render_settings.mutable_underlayer_color() = pb_color;

            pb_render_settings.set_underlayer_width(render_settings_.underlayer_width);

            for (const auto& color : render_settings_.color_palette) {
                tc_pb::Color pb_color;
                visit(RenderSettingsColorVisitor{ pb_color }, color);
                pb_render_settings.mutable_color_palette()->Add(std::move(pb_color));
            }

            *pb_base_.mutable_rendder_settings() = std::move(pb_render_settings);
        }

        void FillTransportRouter() {
            tc_pb::TransportRouter pb_transport_router;

            const graph::DirectedWeightedGraph<BusRouteWeight>& g = transport_router_.GetRouteGraph<BusRouteWeight>();
            for (const auto& edge : g.GetEdges()) {
                tc_pb::Edge pb_edge;
                pb_edge.set_vertex_id_from(edge.from);
                pb_edge.set_vertex_id_to(edge.to);

                tc_pb::BusRouteWeight pb_weight;
                pb_weight.set_span(edge.weight.span);
                pb_weight.set_time(edge.weight.time);

                *pb_edge.mutable_weight() = pb_weight;

                pb_transport_router.mutable_route_graph()->mutable_edges()->Add(std::move(pb_edge));
            }

            for (const std::vector<graph::EdgeId>& incidence_list : g.GetIncidenceLists()) {
                
                tc_pb::IncidenceList pb_incidence_list;

                for (graph::EdgeId edge_id : incidence_list) {
                    pb_incidence_list.add_edge_id(edge_id);
                }

                pb_transport_router.mutable_route_graph()->mutable_incidence_lists()->Add(std::move(pb_incidence_list));
            }

            for (StopPtr stop : transport_router_.GetVertexIndexToStop()) {
                pb_transport_router.add_vertex_index_to_stop(stop->id);
            }
            for (BusPtr bus : transport_router_.GetEdgeIndexToBus()) {
                if (bus) {
                    tc_pb::BusId pb_bus_id;
                    pb_bus_id.set_bus_id(bus->id);
                    pb_bus_id.set_isinitialized(true);
                    pb_transport_router.mutable_edge_index_to_bus()->Add(std::move(pb_bus_id));

                }
                else {
                    tc_pb::BusId pb_bus_id;
                    pb_bus_id.set_isinitialized(false);
                    pb_transport_router.mutable_edge_index_to_bus()->Add(std::move(pb_bus_id));

                }
            }
            *pb_base_.mutable_transport_router() = std::move(pb_transport_router);
        }

        void FillRouter() {

            using RouteInternalData = graph::Router<BusRouteWeight>::RouteInternalData;

            tc_pb::Router pb_router;

            for (const std::vector<std::optional<RouteInternalData>>& row : router_.GetRoutesInternalData()) {

                tc_pb::RouteInternalDataRow pb_row;

                for (const std::optional<RouteInternalData>& data : row) {
                    if (data.has_value()) {
                        tc_pb::RouteInternalData pb_data;

                        pb_data.mutable_weight()->set_span(data->weight.span);
                        pb_data.mutable_weight()->set_time(data->weight.time);

                        if (data->prev_edge.has_value()) {
                            pb_data.mutable_prev_edge()->set_prev_edge_id(data->prev_edge.value());
                        }

                        pb_row.mutable_route_internal_data_row()->Add(std::move(pb_data));

                    }
                    else {
                        pb_row.add_route_internal_data_row();
                    }
                }

                pb_router.mutable_routes_internal_data()->Add(std::move(pb_row));
            }

            *pb_base_.mutable_router() = std::move(pb_router);
        }

    };

    class Deserializer {
    public:
        Deserializer() = delete;
        Deserializer(const std::filesystem::path& p)
            : open_path_(p)
        {
            std::ifstream input_file(open_path_, std::ios::binary);
            if (!input_file) {
                throw std::runtime_error("Can't open file? bad path?");
            }

            pb_base_.ParseFromIstream(&input_file);
        }
        Deserializer(SerializeSettings settings)
            : open_path_(std::filesystem::path(settings.file))
            , serialize_settings_(settings)

        {
            std::ifstream input_file(open_path_, std::ios::binary);
            if (!input_file) {
                throw std::runtime_error("Can't open file? bad path?");
            }

            pb_base_.ParseFromIstream(&input_file);
        }

        catalogue::TransportCatalogue GetTransportCatalogue() const;
        catalogue::RoutingSettings GetRoutingSettings() const;
        renderer::RenderSettings GetRenderSettings() const;

        catalogue::TransportRouter GetTransportRouter(const catalogue::TransportCatalogue& catalogue) const;
        graph::Router<BusRouteWeight> GetRouter(const graph::DirectedWeightedGraph<BusRouteWeight>& graph) const;
    private:
        std::filesystem::path open_path_;

        tc_pb::TransportBase pb_base_;
        SerializeSettings serialize_settings_;

        static svg::Color ExtractSVGColorFromPBColor(tc_pb::Color pb_color);
    };
} // namespace Serialize
