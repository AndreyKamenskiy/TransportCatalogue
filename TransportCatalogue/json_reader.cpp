#include "json_reader.h"

JsonReader::JsonReader(std::istream& input) 
{	
	requests_ = json::Load(input);
}

transport_catalogue::TransportCatalogue JsonReader::create_catalogue()
{
	return transport_catalogue::TransportCatalogue();
}

json::Document JsonReader::get_responce(transport_catalogue::TransportCatalogue)
{
	return json::Document();
}
