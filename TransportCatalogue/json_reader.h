#pragma once
#include <istream>
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"

class JsonReader {
public:
	JsonReader(std::istream& input);
	void add_to_catalogue(transport_catalogue::TransportCatalogue& tc) const;
	json::Document get_responce(const RequestHandler& rh) const ;
private:
	json::Document requests_{ json::Node{} };	
};

