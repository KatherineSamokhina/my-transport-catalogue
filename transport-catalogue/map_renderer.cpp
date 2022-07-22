#include <algorithm>
#include <iterator>

#include "map_renderer.h"

using namespace std;

// ---------- Utilities ------------------    

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point MakePoint(double x, double y) {
    return { x, y };
}

svg::Color MakeColor(const std::string& color) {
    return color;
}

svg::Color MakeColor(int r, int g, int b) {
    return svg::Rgb(r, g, b);
}

svg::Color MakeColor(int r, int g, int b, double a) {
    return svg::Rgba(r, g, b, a);
}

// ---------- SphereProjector ------------------    

SphereProjector::SphereProjector(const geo::Coordinates& left_top,
    const geo::Coordinates& right_bottom,
    double width, double height, double padding)
    : padding_(padding) {
    const double min_lat = left_top.lat;
    max_lat_ = right_bottom.lat;
    min_lon_ = left_top.lng;
    const double max_lon = right_bottom.lng;
    const double width_zoom = (!IsZero(max_lon - min_lon_)) ? (width - 2 * padding_) / (max_lon - min_lon_) : 0;
    const double height_zoom = (!IsZero(max_lat_ - min_lat)) ? (height - 2 * padding_) / (max_lat_ - min_lat) : 0;
    zoom_coef_ = (!IsZero(width_zoom) && !IsZero(height_zoom)) ? min(width_zoom, height_zoom) : max(width_zoom, height_zoom);
}

svg::Point SphereProjector::operator()(const geo::Coordinates& coords) const {
    return { (coords.lng - min_lon_) * zoom_coef_ + padding_,
                (max_lat_ - coords.lat) * zoom_coef_ + padding_ };
}

// ---------- MapRenderer ------------------    

svg::Document MapRenderer::GetDocument() const {
    return document_;
}

void MapRenderer::SetSettings(const Settings& settings) {
    settings_ = settings;
}

Settings MapRenderer::GetSettings() const {
    return settings_;
}

void MapRenderer::SetBorder(const Stops& stops) {
    optional<double> min_lat, max_lat, min_lng, max_lng;
    for (const auto& stop : stops) {
        if (!stop->free) {
            const double lat = stop->coord.lat;
            const double lng = stop->coord.lng;
            min_lat = (min_lat) ? min(*min_lat, lat) : lat;
            max_lat = (max_lat) ? max(*max_lat, lat) : lat;
            min_lng = (min_lng) ? min(*min_lng, lng) : lng;
            max_lng = (max_lng) ? max(*max_lng, lng) : lng;
        }
    }
    if (!min_lat) {
        min_lat = max_lat = min_lng = max_lng = 0;
    }
    sphere_projector_ = SphereProjector({ min_lat.value(), min_lng.value() },
                                        { max_lat.value(), max_lng.value() },
                                        settings_.width, settings_.height, settings_.padding);
}

void MapRenderer::SetTrail(const Buses& buses) {
    for (const auto& bus : buses) {
        RenderTrail(*bus);
    }
    index_color_ = 0;
    for (const auto& bus : buses) {
        RenderTrailName(*bus);
    }
    index_color_ = 0;
}

void MapRenderer::SetStation(const Stops& stops) {
    for (const auto& stop : stops) {
        if (!stop->free) {
            RenderStation(*stop);
        }
    }
    for (const auto& stop : stops) {
        if (!stop->free) {
            RenderStationName(*stop);
        }
    }
}

void MapRenderer::RenderTrail(const Bus& bus) {
    using namespace svg;
    document_.Add(CreateTrail(bus)
        .SetFillColor(NoneColor)
        .SetStrokeColor(GetColor())
        .SetStrokeWidth(settings_.line_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND));
}

void MapRenderer::RenderTrailName(const Bus& bus) {
    using namespace svg;
    const auto& point_begin = GetPoint(bus.stops.front()->coord);
    Text text = Text().SetFillColor(GetColor())
        .SetPosition(point_begin)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s)
        .SetData(bus.number);

    Text underlayer = text;
    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);

    document_.Add(underlayer);
    document_.Add(text);

    const auto& point_end = GetPoint(bus.stops.back()->coord);
    if (!bus.is_ring && point_begin != point_end) {
        text.SetPosition(point_end);
        underlayer.SetPosition(point_end);
        document_.Add(underlayer);
        document_.Add(text);
    }
}

void MapRenderer::RenderStation(const Stop& stop) {
    using namespace svg;
    document_.Add(Circle().SetCenter(GetPoint(stop.coord))
        .SetRadius(settings_.stop_radius)
        .SetFillColor("white"s));
}

void MapRenderer::RenderStationName(const Stop& stop) {
    using namespace svg;
    Text text = Text().SetFillColor("black"s)
        .SetPosition(GetPoint(stop.coord))
        .SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetData(stop.name);

    Text underlayer = text;
    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);

    document_.Add(underlayer);
    document_.Add(text);
}

svg::Polyline MapRenderer::CreateTrail(const Bus& bus) const {
    using namespace svg;
    svg::Polyline line;
    const auto& stops = bus.stops;
    for (auto it = stops.begin(); it != stops.end(); ++it) {
        const auto& point = GetPoint((*it)->coord);
        line.AddPoint(point);
    }

    if (!bus.is_ring) {
        for (auto it = next(stops.rbegin()); it != stops.rend(); ++it) {
            const auto& point = GetPoint((*it)->coord);
            line.AddPoint(point);
        }
    }
    return line;
}

svg::Color MapRenderer::GetColor() {
    return settings_.color_palette.at(GetIndexColor());
}

svg::Point MapRenderer::GetPoint(const geo::Coordinates& coords) const {
    return sphere_projector_(coords);
}

size_t MapRenderer::GetIndexColor() {
    return index_color_++ % settings_.color_palette.size();
}