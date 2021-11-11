#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <string_view>
#include <algorithm>
#include "json_reader.h"


namespace json_requests {
using namespace std::literals::string_literals;
const std::string base_requests = "base_requests"s;
const std::string stat_requests = "stat_requests"s;
const std::string route_type = "Bus"s;
const std::string stop_type = "Stop"s;
const std::string map_type = "Map"s;

const std::string type_key = "type"s;
const std::string name_key = "name"s;
const std::string map_key = "map"s;
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

//for renderer
const std::string render_settings = "render_settings"s;
const std::string render_width = "width"s;
const std::string render_height = "height"s;
const std::string render_padding = "padding"s;
const std::string render_line_width = "line_width"s;
const std::string render_stop_radius = "stop_radius"s;
const std::string render_bus_label_font_size = "bus_label_font_size"s;
const std::string render_bus_label_offset = "bus_label_offset"s;
const std::string render_stop_label_font_size = "stop_label_font_size"s;
const std::string render_stop_label_offset = "stop_label_offset"s;
const std::string render_underlayer_color = "underlayer_color"s;
const std::string render_underlayer_width = "underlayer_width"s;
const std::string render_color_palette = "color_palette"s;

}

JsonReader::JsonReader(std::istream& input)
{	
	requests_ = json::Load(input);
}


void checkDictKey(const json::Dict& map, const std::string& key, const std::type_info& type) {
	using namespace std::literals::string_literals;
	if (map.count(key) == 0) {
		throw std::logic_error("There is no "s + key + " key in the request"s);
	}
	const std::type_info& string_type = typeid(std::string);
	const std::type_info& int_type = typeid(int);
	const std::type_info& double_type = typeid(double);
	const std::type_info& bool_type = typeid(bool);
	const std::type_info& Dict_type = typeid(json::Dict);
	const std::type_info& Array_type = typeid(json::Array);
	if (type.hash_code() == string_type.hash_code()) {
		if (!map.at(key).IsString()) {
			throw std::logic_error("Name "s + key + " is not string type"s); 
		}
	}
	else if (type.hash_code() == int_type.hash_code()) {
			if (!map.at(key).IsInt()) {
				throw std::logic_error("Name "s + key + " is not int type"s);
			}
	}
	else if (type.hash_code() == double_type.hash_code()) {
			if (!map.at(key).IsDouble()) {
				throw std::logic_error("Name "s + key + " is not double type"s);
			}
	}
	else if (type.hash_code() == bool_type.hash_code()) {
			if (!map.at(key).IsBool()) {
				throw std::logic_error("Name "s + key + " is not bool type"s);
			}
	}
	else if (type.hash_code() == Dict_type.hash_code()) {
			if (!map.at(key).IsMap()) {
				throw std::logic_error("Name "s + key + " is not Dict type"s);
			}
	}
	else if (type.hash_code() == Array_type.hash_code()) {
			if (!map.at(key).IsArray()) {
				throw std::logic_error("Name "s + key + " is not Array type"s);
			}
	}
}


void checking_base_request_validity(const json::Node& request) {
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!request.IsMap()) {
		throw std::logic_error("Node is not Dict type"s);
	}
	const json::Dict& map = request.AsMap();
	checkDictKey(map, name_key, typeid(std::string));
	checkDictKey(map, type_key, typeid(std::string));
	if (map.at(type_key) == route_type) {
		checkDictKey(map, circle_route_key, typeid(bool));
		checkDictKey(map, routes_key, typeid(json::Array));
		// валидность имен остановок проверяем не здесь
	}
	else if (map.at(type_key) == stop_type) {
		checkDictKey(map, latitude_key, typeid(double));
		checkDictKey(map, longitude_key, typeid(double));
		checkDictKey(map, longitude_key, typeid(double));
		checkDictKey(map, distances_key, typeid(json::Dict));
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
		if (!distance.IsInt()) {
			using namespace std::literals::string_literals;
			throw std::logic_error("Distant to stop are not an int type"s);
		}
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
		/*if (!map.at(circle_route_key).AsBool()) {
			//если маршрут не цикличный, то мы должны добавить обратный ход
			for (int i = stopNames.size() - 2; i >= 0; --i) {
				stopNames.push_back(stopNames[i]);
			}
		}*/
		tc.addRoute(map.at(name_key).AsString(), stopNames, map.at(circle_route_key).AsBool());
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
	checkDictKey(map, base_requests, typeid(json::Array));

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
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!request.IsMap()) {
		throw std::logic_error("Node is not Dict type"s);
	}
	const json::Dict& map = request.AsMap();
	checkDictKey(map, id_key, typeid(int));
	checkDictKey(map, type_key, typeid(std::string));
	std::string s = map.at(type_key).AsString();
	if (s != route_type && s != stop_type && s != map_type) {
		throw std::logic_error("Unknown request type"s);
	}
	if (s != map_type) {
		checkDictKey(map, name_key, typeid(std::string));
	}
}

json::Document JsonReader::get_responce(const RequestHandler& rh) const
{
	//TODO: make own function for every stat request
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!requests_.GetRoot().IsMap()) {
		throw std::logic_error("Root node is not Dict type"s);
	}
	const json::Dict& map = requests_.GetRoot().AsMap();
	checkDictKey(map, stat_requests, typeid(json::Array));
	const json::Array& stat_requests = map.at(json_requests::stat_requests).AsArray();
	json::Array json_answers;
	for (const json::Node& request : stat_requests) {
		//Is the request valid?
		checking_stat_request_validity(request);
		// prepare request
		const json::Dict& request_dict = request.AsMap();
		int id = request_dict.at(id_key).AsInt();
		json::Dict request_answer;
		request_answer.insert({ request_id_key, json::Node(id) });
		// ask request
		if (request_dict.at(type_key) == stop_type) {
			const std::string& name = request_dict.at(name_key).AsString();
			try {
				const std::unordered_set<domain::Route*>* routes_on_stop = rh.GetBusesByStop(name);
				json::Array routes_array;
				if (routes_on_stop) {
					//если найден хоть один маршрут
					for (const domain::Route* current_route : *routes_on_stop) {
						routes_array.push_back(json::Node{ static_cast<std::string>(current_route->name) });
					}
					//buses — массив названий автобусных маршрутов, проходящих через эту остановку. Названия маршрутов должны быть отсортированы в лексикографическом порядке.
					std::sort(routes_array.begin(), routes_array.end(), [](const json::Node& a, const json::Node& b) { return a.AsString() < b.AsString(); });
				}
				request_answer.insert({ routes_on_stop_key, routes_array });
			}
			catch (std::invalid_argument&) {
				// не найдена остановка с таким именем
				request_answer.insert({ error_message_key, json::Node(not_found_message) });
			}
		}
		else if (request_dict.at(type_key) == route_type) { 
			const std::string& name = request_dict.at(name_key).AsString();
			std::optional<domain::RouteInfo> route_info = rh.GetBusStat(name);
			if (route_info.has_value()) {
				request_answer.insert({ curvature_key, json::Node(route_info->curvature) });
				request_answer.insert({ stop_count_key, json::Node(route_info->stopsNumber) });
				request_answer.insert({ unique_stop_count_key, json::Node(route_info->uniqueStops) });\
				request_answer.insert({ route_length_key, json::Node(route_info->length) });
			}
			else {
				//не нашли такой маршрут
				request_answer.insert({ error_message_key, json::Node(not_found_message) });
			}
		}
		else {
			//map_type
			svg::Document svg = rh.RenderMap();
			std::ostringstream oss;
			svg.Render(oss);
			request_answer.insert({ map_key, json::Node(oss.str()) });
		}
		// add json to the document
		json_answers.push_back(request_answer);
	}
	return json::Document(json::Node(std::move(json_answers)));
}


/*
Цвет можно указать:
1. в виде строки, например, "red" или "black";
2. в массиве из трёх целых чисел диапазона [0, 255]. Они определяют r, g и b компоненты цвета в формате 
svg::Rgb. Цвет [255, 16, 12] нужно вывести как rgb(255, 16, 12);
3. в массиве из четырёх элементов: три целых числа в диапазоне от [0, 255] и одно вещественное число 
в диапазоне от [0.0, 1.0]. Они задают составляющие red, green, blue и opacity цвета формата svg::Rgba. 
Цвет, заданный как [255, 200, 23, 0.85], должен быть выведен как rgba(255, 200, 23, 0.85).
*/
svg::Color node_to_color(const json::Node& node) {
	using namespace std::literals::string_literals;
	svg::Color color;
	if (node.IsString()) {
		return svg::Color{ node.AsString() };
	}
	else if ((node.IsArray())) {
		const json::Array& color_array = node.AsArray();
		if (color_array.size() == 3 || color_array.size() == 4) {
			//todo: need to write a node type check function
			if (!color_array[0].IsInt() || !color_array[1].IsInt() || !color_array[0].IsInt()) {
				throw std::logic_error("The first three elements of the color array must be of type int"s);
			}
			uint8_t r = color_array[0].AsInt();
			uint8_t g = color_array[1].AsInt();
			uint8_t b = color_array[2].AsInt();

			if (color_array.size() == 3) {
				return svg::Color(svg::Rgb{ r,g,b });
			}
			else {
				if (!color_array[3].IsDouble()) {
					throw std::logic_error("The fourth element of the color array must be of type double"s);
				}
				return svg::Color(svg::Rgba(r, g, b, color_array[3].AsDouble()));
			}
		}
		else {
			throw std::logic_error("Color array must consist of 3 or 4 elements"s);
		}
	}
	else
	{
		throw std::logic_error("Unknown type of color node"s);
	}
}

renderer::RenderOptions JsonReader::get_render_options()
{
	using namespace std::literals::string_literals;
	using namespace json_requests;
	if (!requests_.GetRoot().IsMap()) {
		throw std::logic_error("Root node is not Dict type"s);
	}
	const json::Dict& map = requests_.GetRoot().AsMap();
	if (!map.count(render_settings)) {
		//case if there is no render settings in the json
		return {};
	}
	//checkDictKey(map, render_settings, typeid(json::Dict));
	const json::Dict& json_options = map.at(render_settings).AsMap();
	checkDictKey(json_options, render_width, typeid(double));
	checkDictKey(json_options, render_height, typeid(double));
	checkDictKey(json_options, render_padding, typeid(double));
	checkDictKey(json_options, render_line_width, typeid(double));
	checkDictKey(json_options, render_stop_radius, typeid(double));
	checkDictKey(json_options, render_bus_label_font_size, typeid(int));
	checkDictKey(json_options, render_bus_label_offset, typeid(json::Array));
	const json::Array& bus_offset = json_options.at(render_bus_label_offset).AsArray();
	const json::Array& stop_offset = json_options.at(render_stop_label_offset).AsArray();
	if (bus_offset.size() != 2 || stop_offset.size() != 2) {
		throw std::logic_error("label_offset need exactly 2 elements"s);
	}
	if (!bus_offset[0].IsDouble() || !bus_offset[1].IsDouble() 
		|| !stop_offset[0].IsDouble() || !stop_offset[1].IsDouble()) {
		throw std::logic_error("offset must be double type"s);
	}
	checkDictKey(json_options, render_stop_label_font_size, typeid(int));
	checkDictKey(json_options, render_underlayer_width, typeid(double));
	if (json_options.count(render_underlayer_color) == 0) {
		throw std::logic_error("There is no "s + render_underlayer_color + " key in the request"s);
	}
	checkDictKey(json_options, render_color_palette, typeid(json::Array));

	renderer::RenderOptions options;
	options.width = json_options.at(render_width).AsDouble();
	options.height = json_options.at(render_height).AsDouble();
	options.padding = json_options.at(render_padding).AsDouble();
	options.line_width = json_options.at(render_line_width).AsDouble();
	options.stop_radius = json_options.at(render_stop_radius).AsDouble();
	options.bus_label_font_size = json_options.at(render_bus_label_font_size).AsInt();
	options.bus_label_offset.x = bus_offset[0].AsDouble();
	options.bus_label_offset.y = bus_offset[1].AsDouble();
	options.stop_label_offset.x = stop_offset[0].AsDouble();
	options.stop_label_offset.y = stop_offset[1].AsDouble();
	options.stop_label_font_size = json_options.at(render_stop_label_font_size).AsInt();
	options.underlayer_width = json_options.at(render_underlayer_width).AsDouble();
	options.underlayer_color = node_to_color(json_options.at(render_underlayer_color));
	for (const json::Node& current_color : json_options.at(render_color_palette).AsArray()) {
		options.color_palette.push_back(node_to_color(current_color));
	}
	return options;
}