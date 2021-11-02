#pragma once
#include <istream>
#include "json.h"
#include "transport_catalogue.h"

class JsonReader {
public:
	JsonReader(std::istream& input);
	void add_to_catalogue(transport_catalogue::TransportCatalogue& tc);
	json::Document get_responce();
private:
	json::Document requests_{ json::Node{} };	
};

