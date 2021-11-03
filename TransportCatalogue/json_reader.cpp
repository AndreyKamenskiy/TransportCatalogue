#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>
#include "json_reader.h"


namespace json_requests {
using namespace std::literals::string_literals;
const std::string base_requests = "base_requests"s;
const std::string stat_requests = "stat_requests"s;
const std::string route_type = "Bus"s;
const std::string stop_type = "Stop"s;

const std::string type_key = "type"s;
const std::string name_key = "name"s;
const std::string id_key = "id"s; 
const std::string error_message_key = "error_message"s;
const std::string request_id_key = "request_id"s;
const std::string latitude_key = "latitude"s;
const std::string longitude_key = "longitude"s;
const std::string routes_key = "stops"s;
const std::string circle_route_key = "is_roundtrip"s;
const std::string distances_key = "road_distances"s;
const std::string routes_on_stop_key = "buses"s;

const std::string curvature_key = "curvature"s;
const std::string route_length_key = "route_length"s;
const std::string stop_count_key = "stop_count"s;
const std::string unique_stop_count_key = "unique_stop_count"s;

const std::string not_found_message = "not found"s;
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
	const json::Dict& map = request.AsMap();
	if (map.count(name_key) == 0) {
		throw std::logic_error("There is no name key in the request"s);
	}
	if (!map.at(name_key).IsString()) {
		throw std::logic_error("Name key is not string type"s);
	}
	if (map.count(type_key) == 0) {
		throw std::logic_error("There is no type key in the request"s);
	}
	if (!map.at(type_key).IsString()) {
		throw std::logic_error("Type key is not string type"s);
	}
	if (map.at(type_key) == route_type) {
		if (map.count(circle_route_key) == 0) {
			throw std::logic_error("There is no circle route key in the request"s);
		}
		if (!map.at(circle_route_key).IsBool()) {
			throw std::logic_error("Circle route key is not bool type"s);
		}
		if (map.count(routes_key) == 0) {
			throw std::logic_error("There is no routes key in the request"s);
		}
		if (!map.at(routes_key).IsArray()) {
			throw std::logic_error("Routes key is not Array type"s);
		}
		// валидность имен остановок проверяем не здесь
	}
	else if (map.at(type_key) == stop_type) {
		if (map.count(latitude_key) == 0) {
			throw std::logic_error("There is no latitude key in the request"s);
		}
		if (!map.at(latitude_key).IsDouble()) {
			throw std::logic_error("Latitude key is not double type"s);
		}
		if (map.count(longitude_key) == 0) {
			throw std::logic_error("There is no longitude key in the request"s);
		}
		if (!map.at(longitude_key).IsDouble()) {
			throw std::logic_error("Longitude key is not double type"s);
		}
		if (map.count(distances_key) == 0) {
			throw std::logic_error("There is no distances key in the request"s);
		}
		if (!map.at(distances_key).IsMap()) {
			throw std::logic_error("Distance key is not dict type"s);
		}
		// валидность остановок проверяем не здесь, а при их переборе
	}
	else {
		throw std::logic_error("unknown request type"s);
	}
}


void add_distances(transport_catalogue::TransportCatalogue& tc, 
	const domain::Stop* stop, 
	const json::Dict& distances) 
{
	for (auto& [toStopName, distance] : distances) {
		const domain::Stop* toStop;
		if (tc.hasStop(toStopName)) {
			toStop = tc.findStop(toStopName);
		}
		else {
			// добавим остановку с нулевыми координатами.
			//TODO: addStop must return pointer to Stop.
			tc.addStop(toStopName);
			toStop = tc.findStop(toStopName);
		}
		// добавим дистанцию до остановки
		tc.addStopsDistance(stop, toStop, distance.AsInt());
	}
}

void add_routes(transport_catalogue::TransportCatalogue& tc,
	std::vector<const json::Node*>& add_route_requests) {
	using namespace json_requests;
	for (const json::Node* request : add_route_requests) {
		auto& map = request->AsMap();
		std::vector<std::string_view> stopNames;
		for (auto& stop : map.at(routes_key).AsArray()) {
			if (!stop.IsString()) {
				throw std::logic_error("Stop name is not string type"s);
			}
			stopNames.push_back(stop.AsString());
		}
		if (!map.at(circle_route_key).AsBool()) {
			//если маршрут не цикличный, то мы должны добавить обратный ход
			for (int i = stopNames.size() - 2; i >= 0; --i) {
				stopNames.push_back(stopNames[i]);
			}
		}
		tc.addRoute(map.at(name_key).AsString(), stopNames);
	}
}


void JsonReader::add_to_catalogue(transport_catalogue::TransportCatalogue& tc) const
{
	using namespace std::literals::string_literals;
	using namespace json_requests;
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
	std::vector<const json::Node*> add_route_requests;
	for (const json::Node& request : base_requests) {
		checking_base_request_validity(request);
		const json::Dict& request_dict = request.AsMap();
		if (request_dict.at(type_key) == stop_type) {
			const std::string& name = request_dict.at(name_key).AsString();
			domain::Coordinates coordinates{
				request_dict.at(latitude_key).AsDouble(),
				request_dict.at(longitude_key).AsDouble()
			};
			//todo: change addStop to update coordinates if the stop already exist
			if (tc.hasStop(name)) {
				//если такая остановка встречалась ранее, то обновим координаты
				tc.updateStopCoordinates(name, coordinates);
			}
			else {
				tc.addStop(name, coordinates);
			}
			const domain::Stop* stop = tc.findStop(name);
			add_distances(tc, stop, request_dict.at(distances_key).AsMap());
		}
		else { // можем не проверять на другие виды запросов, т.к. мы их отсекли при проверке валидации
			//route_type
			add_route_requests.push_back(&request);
		}
	}
	add_routes(tc, add_route_requests);
}

void checking_stat_request_validity(const json::Node& request) {
	//todo: сократить повторяющийся код
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!request.IsMap()) {
		throw std::logic_error("Node is not Dict type"s);
	}
	const json::Dict& map = request.AsMap();
	//cheching id
	if (map.count(id_key) == 0) {
		throw std::logic_error("There is no id key in the request"s);
	}
	if (!map.at(id_key).IsInt()) {
		throw std::logic_error("Id key is not int type"s);
	}
	//cheching name
	if (map.count(name_key) == 0) {
		throw std::logic_error("There is no name key in the request"s);
	}
	if (!map.at(name_key).IsString()) {
		throw std::logic_error("Name key is not string type"s);
	}
	//cheching type
	if (map.count(type_key) == 0) {
		throw std::logic_error("There is no type key in the request"s);
	}
	if (!map.at(type_key).IsString()) {
		throw std::logic_error("Type key is not string type"s);
	}
	if (map.at(type_key).AsString() != route_type && map.at(type_key).AsString() != stop_type) {
		throw std::logic_error("Unknown request type"s);
	}
}




json::Document JsonReader::get_responce(const RequestHandler& rh) const
{
	//todo: move blocks of code to the functions. Make method smaller.

	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!requests_.GetRoot().IsMap()) {
		throw std::logic_error("Root node is not Dict type"s);
	}
	const json::Dict& map = requests_.GetRoot().AsMap();
	if (map.count(stat_requests) == 0) {
		throw std::logic_error("Stat requests are missing"s);
	}
	if (!map.at(stat_requests).IsArray()) {
		throw std::logic_error("Stat requests are not an array type"s);
	}
	const json::Array& stat_requests = map.at(json_requests::stat_requests).AsArray();
	json::Array json_answers;
	for (const json::Node& request : stat_requests) {
		//Is the request valid?
		checking_stat_request_validity(request);
		// prepare request
		const json::Dict& request_dict = request.AsMap();
		int id = request_dict.at(id_key).AsInt();
		const std::string& name = request_dict.at(name_key).AsString();
		json::Dict request_answer;
		request_answer.insert({ request_id_key, json::Node(id) });
		// ask request
		if (request_dict.at(type_key) == stop_type) {
			try {
				const std::unordered_set<domain::Route*>* routes_on_stop = rh.GetBusesByStop(name);
				json::Array routes_array;
				for (const domain::Route* current_route : *routes_on_stop) {
					routes_array.push_back(json::Node{ static_cast<std::string>(current_route->name) });
				}
				request_answer.insert({ routes_on_stop_key, routes_array });
			}
			catch (std::invalid_argument&) {
				// не найдена остановка с таким именем
				request_answer.insert({ error_message_key, json::Node(not_found_message) });
			}
		}
		else { // можем не проверять на другие виды запросов, т.к. мы их отсекли при проверке валидации
			//route_type
			std::optional<domain::RouteInfo> route_info = rh.GetBusStat(name);
			if (route_info.has_value()) {
				request_answer.insert({ curvature_key, json::Node(route_info->curvature) });
				request_answer.insert({ stop_count_key, json::Node(route_info->stopsNumber) });
				request_answer.insert({ unique_stop_count_key, json::Node(route_info->uniqueStops) });\
				//todo: не забыть убрать после изменения domain::RouteInfo.length на int
				int len = static_cast<int>(route_info->length);
				request_answer.insert({ route_length_key, json::Node(len) });
			}
			else {
				//не нашли такой маршрут
				request_answer.insert({ error_message_key, json::Node(not_found_message) });
			}
		}
		// add json to the document
		json_answers.push_back(request_answer);
	}
	return json::Document(json::Node(std::move(json_answers)));
}
