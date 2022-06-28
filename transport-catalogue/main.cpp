#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"

int main() {
    catalog::input_utils::ParseTransportCatalogueQueries(std::cin);
}