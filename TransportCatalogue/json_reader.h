#pragma once
#include <istream>
#include "json.h"
#include "transport_catalogue.h"

class JsonReader {
public:
	JsonReader(std::istream& input);
	transport_catalogue::TransportCatalogue create_catalogue();
	json::Document get_responce(transport_catalogue::TransportCatalogue);
private:
	json::Document requests_{ { nullptr } };
};

