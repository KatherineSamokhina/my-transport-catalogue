#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

namespace catalogue::input_utils {

void RemoveSpaces(std::string& str);

DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text);

std::pair<catalogue::Stop, bool> ParseBusStopInput(const std::string& text);

catalog::Bus ParseBusRouteInput(std::string_view text);

void ParseTransportCatalogueQueries(std::istream& input_stream);

}  // namespace catalog::input_utils