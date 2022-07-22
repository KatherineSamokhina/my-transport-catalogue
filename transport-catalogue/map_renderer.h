#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>

#include "domain.h"
#include "geo.h"
#include "svg.h"

inline const double EPSILON = 1e-6;

struct Settings {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    uint64_t bus_label_font_size = 0;
    svg::Point bus_label_offset;
    uint64_t stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0;
    std::vector<svg::Color> color_palette;
};

bool IsZero(double value);
svg::Point MakePoint(double x, double y);
svg::Color MakeColor(const std::string& color);
svg::Color MakeColor(int r, int g, int b);
svg::Color MakeColor(int r, int g, int b, double a);

class SphereProjector final {
    double padding_;
    double min_lon_;
    double max_lat_;
    double zoom_coef_ = 0;

public:
    SphereProjector() = default;

    SphereProjector(const geo::Coordinates& left_top,
        const geo::Coordinates& right_bottom,
        double width, double height, double padding);

    svg::Point operator()(const geo::Coordinates& coords) const;
};

class MapRenderer final {
    SphereProjector sphere_projector_;
    Settings settings_;
    size_t index_color_ = 0;
    svg::Document document_;

public:
    MapRenderer() = default;

    svg::Document GetDocument() const;

    void SetSettings(const Settings& settings);
    Settings GetSettings() const;

    void SetBorder(const Stops& stops);
    void SetTrail(const Buses& buses);
    void SetStation(const Stops& stops);

private:
    void RenderTrail(const Bus& bus);
    void RenderTrailName(const Bus& bus);
    void RenderStation(const Stop& stop);
    void RenderStationName(const Stop& stop);

    svg::Polyline CreateTrail(const Bus& bus) const;
    svg::Color GetColor();
    svg::Point GetPoint(const geo::Coordinates& coords) const;
    size_t GetIndexColor();
};