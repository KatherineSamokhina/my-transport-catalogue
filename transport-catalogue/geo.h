#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
    double lat = 0.0;
    double lng = 0.0;

    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo