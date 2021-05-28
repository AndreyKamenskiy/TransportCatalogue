#include "transport_catalogue.h"
#include <stdexcept>
#include <unordered_set>

//добавить остановку в базу
void TransportCatalogue::addStop(std::string_view name, Coordinates coordinates) {
	std::string_view nameSV = addString(name);
	stops_.push_back({nameSV, coordinates});
	nameToStop_[nameSV] = &stops_.back();
}

//поиск остановки по имени
const Stop* TransportCatalogue::findStop(std::string_view name) {
	if (nameToStop_.count(name) == 0) {
		return nullptr; // возможно лучше заменить на исключение.
	}
	return nameToStop_.at(name);
}

// добавление маршрута в базу
void TransportCatalogue::addRoute(std::string_view name, std::vector<std::string_view>& stopNames ) {
	std::vector<const Stop*> stops;
	stops.reserve(stopNames.size());
	for (auto& stop : stopNames) {
		const Stop* stopPtr = findStop(stop);
		if (!stopPtr) {
			using namespace std::string_literals;
			throw std::invalid_argument("there is no such stop in the database: '"s + static_cast<std::string>(stop) + "'"s);
		}
		stops.push_back(stopPtr);
	}
	std::string_view nameSV = addString(name);
	routes_.push_back({ nameSV, stops });
	nameToRoute_[nameSV] = &routes_.back();
}


//поиск маршрута по имени
const Route* TransportCatalogue::findRoute(std::string_view name) {
	if (nameToRoute_.count(name) == 0) {
		return nullptr; // возможно лучше заменить на исключение.
	}
	return nameToRoute_.at(name);
}

const RouteInfo TransportCatalogue::getRouteInfo(const Route* route) {
	if (!route) {
		using namespace std::string_literals;
		throw std::invalid_argument("invalid route pointer"s);
	}
	std::unordered_set<std::string_view> unique;
	//get Length and uniques of the route;
	double length = 0;
	const Stop* previous = nullptr;
	for (const Stop* current : route->stops) {
		unique.insert(current->name);
		if (previous) {
			length += ComputeDistance(previous->coordinates, current->coordinates);
		}
		previous = current;
	}
	return { static_cast<int>(route->stops.size()), static_cast<int>(unique.size()), length };
}

const RouteInfo TransportCatalogue::getRouteInfo(const std::string_view routeName) {
	return	getRouteInfo(findRoute(routeName));
}
