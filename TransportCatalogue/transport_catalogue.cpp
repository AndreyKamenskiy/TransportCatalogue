#include "transport_catalogue.h"
#include <stdexcept>

//добавить остановку в базу
void TransportCatalogue::addStop(std::string& name, Coordinates coordinates) {
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
void TransportCatalogue::addRoute(std::string& name, RouteType type, std::vector<std::string>& stopNames ) {
	std::vector<const Stop*> stops;
	stops.reserve(stopNames.size());
	for (auto& stop : stopNames) {
		const Stop* stopPtr = findStop(stop);
		if (!stopPtr) {
			using namespace std::string_literals;
			throw std::invalid_argument("there is no such stop in the database: "s + stop);
		}

		stops.push_back(stopPtr);
	}

	std::string_view nameSV = addString(name);
	routes_.push_back({ nameSV, type, stops });
	auto* newRoute = &routes_.back();
	for (const Stop* stop : stops) {
		stopToRoutes_[stop].push_back(newRoute);
	}
	nameToRoute_[nameSV] = newRoute;
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
	int unique = 0;
	double length;

	return { route->stops.size(), unique, length };
}

const RouteInfo TransportCatalogue::getRouteInfo(const std::string_view routeName) {
	return	getRouteInfo(findRoute(routeName));
}
