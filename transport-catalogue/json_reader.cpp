#include "json_reader.h"

using namespace std::literals;

namespace json_reader {
    void JsonReader::ReadBaseRequests(json::Document document) {
        json::Array base_requests = document.GetRoot().AsDict().at("base_requests").AsArray();
        for (const json::Node& base_request : base_requests) {
            assert(base_request.IsDict());
            assert(base_request.AsDict().count("type") != 0);
            if (base_request.AsDict().at("type").AsString() == "Stop") {
                AddStopBaseRequest(base_request);
            }
            else if (base_request.AsDict().at("type").AsString() == "Bus") {
                AddBusBaseRequest(base_request);
            }
            else {
                throw std::exception();
            }
        }
    }

    void JsonReader::AddStopBaseRequest(const json::Node& request) {
        AddStopRequest add_stop_request{};
        add_stop_request.cordinates.lat = request.AsDict().at("latitude").AsDouble();
        add_stop_request.cordinates.lng = request.AsDict().at("longitude").AsDouble();
        add_stop_request.name = request.AsDict().at("name").AsString();
        const json::Dict& road_distances = request.AsDict().at("road_distances").AsDict();
        for (const auto& road_distance : road_distances) {
            add_stop_request.distances[road_distance.first] = road_distance.second.AsInt();
        }
        add_stop_requests_.push_back(std::move(add_stop_request));
    }
    void JsonReader::AddBusBaseRequest(const json::Node& request) {
        AddBusRequest add_bus_request{};
        add_bus_request.name = request.AsDict().at("name").AsString();
        add_bus_request.is_roundtrip = request.AsDict().at("is_roundtrip").AsBool();
        const json::Array& stops = request.AsDict().at("stops").AsArray();
        for (const auto& stop : stops) {
            add_bus_request.stops.push_back(std::move(stop.AsString()));
        }
        add_bus_requests_.push_back(std::move(add_bus_request));
    }

    renderer::RenderSettings JsonReader::ReadRenderSettingsFromJSON(const json::Document& document) const {
        renderer::RenderSettings settings;
        assert(document.GetRoot().IsDict());
        assert(document.GetRoot().AsDict().count("render_settings") != 0);
        const json::Node& json_settings = document.GetRoot().AsDict().at("render_settings").AsDict();

        settings.bus_label_font_size = json_settings.AsDict().at("bus_label_font_size").AsInt();
        settings.bus_label_offset.x = json_settings.AsDict().at("bus_label_offset").AsArray()[0].AsDouble();
        settings.bus_label_offset.y = json_settings.AsDict().at("bus_label_offset").AsArray()[1].AsDouble();

        for (const json::Node& color : json_settings.AsDict().at("color_palette").AsArray()) {
            settings.color_palette.push_back(ReadColor(color));
        }

        settings.height = json_settings.AsDict().at("height").AsDouble();
        settings.line_width = json_settings.AsDict().at("line_width").AsDouble();
        settings.padding = json_settings.AsDict().at("padding").AsDouble();
        settings.stop_label_font_size = json_settings.AsDict().at("stop_label_font_size").AsInt();
        settings.stop_label_offset.x = json_settings.AsDict().at("stop_label_offset").AsArray()[0].AsDouble();
        settings.stop_label_offset.y = json_settings.AsDict().at("stop_label_offset").AsArray()[1].AsDouble();
        settings.stop_radius = json_settings.AsDict().at("stop_radius").AsDouble();
        settings.underlayer_color = ReadColor(json_settings.AsDict().at("underlayer_color"));
        settings.underlayer_width = json_settings.AsDict().at("underlayer_width").AsDouble();
        settings.width = json_settings.AsDict().at("width").AsDouble();

        return settings;
    }

    svg::Color JsonReader::ReadColor(const json::Node& node) const {
        svg::Color color;
        if (std::holds_alternative<json::Array>(node.GetValue())) {
            switch (node.AsArray().size())
            {
            case 3:
                color = svg::Rgb{
                    static_cast<unsigned char>(node.AsArray()[0].AsInt()),
                    static_cast<unsigned char>(node.AsArray()[1].AsInt()),
                    static_cast<unsigned char>(node.AsArray()[2].AsInt())
                };
                break;
            case 4:
                color = svg::Rgba{
                    static_cast<unsigned char>(node.AsArray()[0].AsInt()),
                    static_cast<unsigned char>(node.AsArray()[1].AsInt()),
                    static_cast<unsigned char>(node.AsArray()[2].AsInt()),
                    node.AsArray()[3].AsDouble()
                };
                break;
            default:
                break;
            };
        }
        else if (std::holds_alternative<std::string>(node.GetValue())) {
            color = node.AsString();
        }
        else {
            throw std::bad_variant_access();
        }
        return color;
    }

    JsonReader::JsonReader(json::Document document)
        : document_(document)
    {
        if (document.GetRoot().AsDict().count("base_requests")) {
            ReadBaseRequests(document);
        }
    }

    renderer::RenderSettings JsonReader::GetRenderSettings() const {
        return ReadRenderSettingsFromJSON(document_);
    }

    json::Document JsonReader::ProcessStatRequests(RequestHandler& handler) {
        json::Array answers_array{};

        json::Array stat_requests = document_.GetRoot().AsDict().at("stat_requests").AsArray();

        for (const auto& stat_request : stat_requests) {
            std::string_view request_type = stat_request.AsDict().at("type").AsString();
            if (request_type == "Bus"sv) {

                ProcessBusStatRequest(handler, stat_request, answers_array);

            }
            else if (request_type == "Stop"sv) {

                ProcessStopInfoRequest(handler, stat_request, answers_array);

            }
            else if (request_type == "Map"sv) {

                ProcessMapRequest(handler, stat_request, answers_array);

            }
            else if (request_type == "Route"sv) {

                ProcessRouteRequest(handler, stat_request, answers_array);

            }
            else {
                throw std::logic_error("bad stat request");
            }
        }

        json::Document document{ answers_array };
        return document;
    }

    void JsonReader::ProcessBusStatRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
        std::optional<BusInfo> bus_stat = handler.GetBusStat(stat_request.AsDict().at("name").AsString());
        int id = stat_request.AsDict().at("id").AsInt();
        answers_array.push_back(std::move(ConvertBusStatToJsonDict(id, bus_stat)));
    }

    void JsonReader::ProcessStopInfoRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
        int id = stat_request.AsDict().at("id").AsInt();
        std::optional<StopInfo> stop_info = handler.GetStopInfo(stat_request.AsDict().at("name").AsString());
        answers_array.push_back(std::move(ConvertStopInfoToJsonDict(id, stop_info)));
    }

    void JsonReader::ProcessMapRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
        int id = stat_request.AsDict().at("id").AsInt();
        std::string map_as_string = std::move(handler.RenderMap());
        answers_array.push_back(std::move(ConvertMapToJsonDict(id, map_as_string)));
    }

    json::Node JsonReader::ConvertBusStatToJsonDict(int id, std::optional<BusInfo> bus_stat) {

        if (bus_stat.has_value()) {
            // автобус существует
            auto stat = bus_stat.value();
            json::Node answer =
                json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("curvature").Value(stat.curvature)
                .Key("unique_stop_count").Value(static_cast<int>(stat.unique_stops_count))
                .Key("stop_count").Value(static_cast<int>(stat.stops_count))
                .Key("route_length").Value(static_cast<double>(stat.route_length))
                .EndDict().Build();

            return answer;
        }
        else {
            json::Node answer{
                json::Builder{}.StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                    .EndDict().Build()
            };
            return answer;
        }
    }

    json::Node JsonReader::ConvertStopInfoToJsonDict(int id, std::optional<StopInfo> stop_info) {
        if (stop_info.has_value()) {
            auto info = stop_info.value();
            json::Array buses;
            for (const auto bus : info.buses_) {
                buses.push_back(std::move(std::string(bus)));
            }
            json::Node answer{
                json::Builder{}.StartDict()
                    .Key("request_id").Value(id)
                    .Key("buses").Value(buses)
                .EndDict().Build()
            };
            return answer;
        }
        else {
            json::Node answer{
                json::Builder{}.StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                    .EndDict().Build()
            };
            return answer;
        }
    }

    json::Node JsonReader::ConvertMapToJsonDict(int id, std::string map_as_string) {
        json::Node answer{
                json::Builder{}.StartDict()
                    .Key("request_id").Value(id)
                    .Key("map").Value(std::move(map_as_string))
                .EndDict().Build()
        };
        return answer;
    }

    catalogue::RoutingSettings JsonReader::ReadRoutingSettings(const json::Document& document) const {
        catalogue::RoutingSettings settings;
        assert(document.GetRoot().IsDict());
        assert(document.GetRoot().AsDict().count("routing_settings") != 0);
        const json::Node& json_settings = document.GetRoot().AsDict().at("routing_settings").AsDict();
        settings.bus_wait_time = json_settings.AsDict().at("bus_wait_time").AsInt();
        settings.bus_velocity = json_settings.AsDict().at("bus_velocity").AsInt() * 100.0 / 6.0;
        return settings;
    }

    void JsonReader::ProcessRouteRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
        int id = stat_request.AsDict().at("id").AsInt();
        std::string stop_from = stat_request.AsDict().at("from").AsString();
        std::string stop_to = stat_request.AsDict().at("to").AsString();
        std::optional<graph::Router<BusRouteWeight>::RouteInfo> route_info = handler.GetRouteInfo(stop_from, stop_to);
        answers_array.push_back(std::move(ConvertRouteInfoToJsonDict(id, route_info, handler)));
    }

    json::Node JsonReader::ConvertRouteInfoToJsonDict(int id,
        std::optional<graph::Router<BusRouteWeight>::RouteInfo> route_info,
        RequestHandler& handler) {
        json::Node answer = json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .EndDict()
            .Build();

        if (route_info.has_value()) {
            answer.AsDict().emplace("total_time", route_info->weight.time);
            answer.AsDict().emplace("items", json::Array{});
            json::Array& items = answer.AsDict().at("items").AsArray();
            for (const graph::EdgeId edge_id : route_info->edges) {
                json::Dict item{};
                const graph::Edge<BusRouteWeight>& edge = handler.GetEdgeByIndex(edge_id);

                double time = edge.weight.time;
                item.emplace("time", time);

                if (auto bus = handler.GetBusByEdgeIndex(edge_id)) {
                    item.emplace("type", "Bus");
                    std::string bus_name = bus->name_;
                    item.emplace("bus", bus_name);
                    item.emplace("span_count", edge.weight.span);

                }
                else {
                    item.emplace("type", "Wait");
                    item.emplace(
                        "stop_name",
                        handler.GetStopByVertexIndex(edge.from)->name_
                    );
                }
                items.push_back(std::move(item));
            }

        }
        else {
            answer.AsDict().emplace("error_message", "not found");
        }
        return answer;
    }

    void JsonReader::Fill(catalogue::TransportCatalogue& catalogue, catalogue::TransportRouter& router) {
        for (const auto& [name, coordinates, _] : add_stop_requests_) {
            catalogue.AddStop(name, coordinates);
        }
        for (const auto& [name_from, _, distances] : add_stop_requests_) {
            StopPtr stop_from = catalogue.FindStop(name_from);
            for (const auto& [name_to, distance] : distances) {
                StopPtr stop_to = catalogue.FindStop(name_to);
                catalogue.SetDistance({ stop_from, stop_to }, distance);
            }
            router.AddStopVertex(stop_from);
        }

        router.AddBusWaitEdges();

        for (const auto& [name, stops, type] : add_bus_requests_) {
            BusType bus_type;
            if (type) {
                bus_type = BusType::CYCLED;
            }
            else {
                bus_type = BusType::ORDINARY;
            }
            catalogue.AddBus(name, stops, bus_type);
            router.AddBusEdges(name);
        }
    }

    Serialize::SerializeSettings JsonReader::ReadSerializeSettings(const json::Document& document) const {
        Serialize::SerializeSettings settings;
        assert(document.GetRoot().IsDict());
        assert(document.GetRoot().AsDict().count("serialization_settings") != 0);
        settings.file = document.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
        return settings;
    }
}
