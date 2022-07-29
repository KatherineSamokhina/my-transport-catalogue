#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

int main() {
	catalog::TransportCatalogue base;
	MapRenderer renderer;
	RequestHandler request(base, renderer);
	JSONreader reader(base, renderer, request);

	const json::Document doc = json::Load(std::cin);
	const auto& mode = doc.GetRoot().AsDict();

	if (mode.count("base_requests") || mode.count("render_settings")) {
		reader.ReadRequest(doc);
	}
	if (mode.count("stat_requests")) {
		reader.ReadRequests(doc);
	}

	reader.ReadTransportCatalogue();

	return 0;
}