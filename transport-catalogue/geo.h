#pragma once

namespace geo {

    static const double EARTH_RADIUS = 6371000;

    struct Coordinates {
        double lat = 0.0;
        double lng = 0.0;
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo