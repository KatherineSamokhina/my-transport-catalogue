#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <iostream>
#include <string_view>
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
     if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    if (mode == "make_base"sv) {
        {
            json::Document doc = json::Load(std::cin);
            json_reader::JsonReader reader(doc);

            catalogue::TransportCatalogue cat;
            catalogue::TransportRouter transport_router(reader.ReadRoutingSettings(doc), cat);
            reader.Fill(cat, transport_router);
            graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

            Serialize::Serializer serializer(cat, transport_router, reader.GetRenderSettings(), reader.ReadSerializeSettings(doc), router);
            serializer.Save();
        }

    } else if (mode == "process_requests"sv) {
        {
            json::Document doc = json::Load(std::cin);
            json_reader::JsonReader reader(doc);

            Serialize::Deserializer deserializer(reader.ReadSerializeSettings(doc));
            catalogue::TransportCatalogue cat = deserializer.GetTransportCatalogue();
            catalogue::TransportRouter transport_router = deserializer.GetTransportRouter(cat);

            renderer::MapRenderer renderer(deserializer.GetRenderSettings(), cat.GetBusesSorted());
            graph::Router<BusRouteWeight> router = deserializer.GetRouter(transport_router.GetRouteGraph<BusRouteWeight>());
            RequestHandler handler(cat, renderer, router, transport_router);
            json::Document result = reader.ProcessStatRequests(handler);
            json::Print(result, std::cout);
        }

    } else {
         PrintUsage();
         return 1;
    }
}