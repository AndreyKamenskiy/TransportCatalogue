#include "json_reader.h"
#include <stdexcept>
#include <string>


namespace json_requests {
using namespace std::literals::string_literals;
const std::string base_requests = "base_requests"s;
const std::string stat_requests = "stat_requests"s;
const std::string route_type = "Bus"s;
const std::string stop_type = "Stop"s;

const std::string type_key = "type"s;
const std::string name_key = "name"s;
const std::string latitude_key = "latitude"s;
const std::string longitude_key = "longitude"s;
const std::string routes_key = "stops"s;
const std::string circle_route_key = "is_roundtrip"s;
const std::string distances_key = "road_distances"s;
}

JsonReader::JsonReader(std::istream& input) 
{	
	requests_ = json::Load(input);
}

void checking_base_request_validity(const json::Node& request) {
	//todo: сократить повторяющийся код
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!request.IsMap()) {
		throw std::logic_error("Node is not Dict type"s);
	}
	if (!request.IsMap()) {
		throw std::logic_error("Node is not Dict type"s);
	}
	const json::Dict& map = request.AsMap();
	if (map.count(name_key) == 0) {
		throw std::logic_error("There is no name key in the request"s);
	}
	if (map.count(type_key) == 0) {
		throw std::logic_error("There is no type key in the request"s);
	}
	if (map.at(type_key) == route_type) {
		if (map.count(circle_route_key) == 0) {
			throw std::logic_error("There is no circle route key in the request"s);
		}
		if (!map.at(circle_route_key).IsBool()) {
			throw std::logic_error("Circle route key in not bool type"s);
		}
		if (map.count(routes_key) == 0) {
			throw std::logic_error("There is no routes key in the request"s);
		}
		if (!map.at(routes_key).IsArray()) {
			throw std::logic_error("Routes key in not Array type"s);
		}
		// валидность имен остановок проверяем не здесь
	}
	else if (map.at(type_key) == stop_type) {
		if (map.count(latitude_key) == 0) {
			throw std::logic_error("There is no latitude key in the request"s);
		}
		if (!map.at(latitude_key).IsDouble()) {
			throw std::logic_error("Latitude key in not double type"s);
		}
		if (map.count(longitude_key) == 0) {
			throw std::logic_error("There is no longitude key in the request"s);
		}
		if (!map.at(longitude_key).IsDouble()) {
			throw std::logic_error("Longitude key in not double type"s);
		}
		if (map.count(distances_key) == 0) {
			throw std::logic_error("There is no distances key in the request"s);
		}
		if (!map.at(distances_key).IsMap()) {
			throw std::logic_error("Distance key in not dict type"s);
		}
		// валидность остановок проверяем не здесь, а при их переборе
	}
	else {
		throw std::logic_error("unknown request type"s);
	}
}

transport_catalogue::TransportCatalogue JsonReader::create_catalogue()
{
	using namespace std::literals;
	//Проверим есть ли в загруженном JSON документе запросы добавления к базе в нужном формате
	if (!requests_.GetRoot().IsMap()) {
		throw std::logic_error("Root node is not Dict type"s);
	}
	const json::Dict& map = requests_.GetRoot().AsMap();
	if (map.count(json_requests::base_requests) == 0) {
		throw std::logic_error("Base requests are missing"s);
	}
	if (!map.at(json_requests::base_requests).IsArray()) {
		throw std::logic_error("Base requests are not an array type"s);
	}
	const json::Array& base_requests = map.at(json_requests::base_requests).AsArray();
	for (const json::Node& request : base_requests) {
		checking_base_request_validity(request);
	}



			
	return transport_catalogue::TransportCatalogue();
}

json::Document JsonReader::get_responce(transport_catalogue::TransportCatalogue)
{
	return json::Document({nullptr});
}
