#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <optional>
#include "transport_catalogue.h"

using namespace transport_catalogue;

//добавить остановку в базу
void TransportCatalogue::addStop(std::string_view name, Coordinates coordinates) {
	std::string_view nameSV = addString(name);
	stops_.push_back({ nameSV, coordinates });
	nameToStop_[nameSV] = &stops_.back();
}

//добавить остановку без координат. Координаты добавляются позже чкрез updateCoordinates;
//Нужно для того, чтобы добавить остановку тогда, когда она встретилась впервый раз.
void TransportCatalogue::addStop(std::string_view name)
{
	std::string_view nameSV = addString(name);
	stops_.push_back({ nameSV, voidCoordinates });
	nameToStop_[nameSV] = &stops_.back();
}

// обновить координаты остановки. Сделано для того, чтобы при появлении информации о координатах 
// остановки добавить их.
void TransportCatalogue::updateStopCoordinates(std::string_view name, Coordinates coordinates) {
	if (nameToStop_.count(name) == 0) {
		using namespace std::string_literals;
		throw std::invalid_argument("there is no such stop in the database: '"s + static_cast<std::string>(name) + "'"s);
	}
	nameToStop_.at(name)->coordinates = coordinates;
}

//поиск остановки по имени
const Stop* TransportCatalogue::findStop(std::string_view name) const {
	if (nameToStop_.count(name) == 0) {
		using namespace std::string_literals;
		throw std::invalid_argument("there is no such stop in the database: '"s + static_cast<std::string>(name) + "'"s);
	}
	return nameToStop_.at(name);
}

bool TransportCatalogue::hasStop(std::string_view name) const
{
	return nameToStop_.count(name) != 0;
}

void TransportCatalogue::addStopsDistance(const Stop* stopA, const Stop* stopB, int distance)
{
	stopsDistance[std::pair<const Stop*, const Stop*> {stopA, stopB}] = distance;
}

// добавление маршрута в базу
void TransportCatalogue::addRoute(std::string_view name, std::vector<std::string_view>& stopNames, bool isCircle) {
	std::vector<const Stop*> stops;
	stops.reserve(stopNames.size());
	std::optional<const Stop*> firstFinalStop;
	std::optional<const Stop*> secondFinalStop;
	for (auto& stop : stopNames) {
		const Stop* stopPtr = findStop(stop);
		if (!stopPtr) {
			using namespace std::string_literals;
			throw std::invalid_argument("there is no such stop in the database: '"s + static_cast<std::string>(stop) + "'"s);
		}
		stops.push_back(stopPtr);
	}
	if (stopNames.size() > 0) {
		firstFinalStop = stops[0];
	}
	if (!isCircle && stopNames.size() > 0) {
		secondFinalStop = stops.back();
		//если маршрут не цикличный, то мы должны добавить обратный ход
		for (int i = stops.size() - 2; i >= 0; --i) {
			stops.push_back(stops[i]);
		}
	}
	std::string_view nameSV = addString(name);
	routes_.push_back({ nameSV, stops, firstFinalStop, secondFinalStop });
	nameToRoute_[nameSV] = &routes_.back();
	for (auto stopPtr : stops) {
		stopToRoutes_[stopPtr].insert(&routes_.back());
	}
}

//поиск маршрута по имени
const Route* TransportCatalogue::findRoute(std::string_view name) const {
	if (nameToRoute_.count(name) == 0) {
		return nullptr; // возможно лучше заменить на исключение.
	}
	return nameToRoute_.at(name);
}

const RouteInfo TransportCatalogue::getRouteInfo(const Route* route) const {
	if (!route) {
		using namespace std::string_literals;
		throw std::invalid_argument("invalid route pointer"s);
	}
	std::unordered_set<std::string_view> unique;
	//get Length and uniques of the route;
	//todo:make length int type
	double length = 0;
	double geoLength = 0;
	const Stop* previous = nullptr;
	for (const Stop* current : route->stops) {
		unique.insert(current->name);
		if (previous) {
			geoLength += ComputeDistance(previous->coordinates, current->coordinates);
			length += getRealStopsDistance(previous, current);
		}
		previous = current;
	}
	return { static_cast<int>(route->stops.size()), static_cast<int>(unique.size()), length, length / geoLength };
}

const RouteInfo TransportCatalogue::getRouteInfo(const std::string_view& routeName) const {
	return	getRouteInfo(findRoute(routeName));
}

//Поиск маршрутов по остановке 
size_t TransportCatalogue::getRoutesNumOnStop(const Stop* stop) const {
	return stopToRoutes_.count(stop);
}

const std::unordered_set<domain::Route*>* TransportCatalogue::getRoutesOnStop(const Stop* stop) const {
	//return { stopToRoutes_.at(stop).begin(), stopToRoutes_.at(stop).end() };
	if (stopToRoutes_.count(stop) == 0) {
		return nullptr; // если через остановку не проходит ни один автобус, вернем nullptr
	}
	return  &stopToRoutes_.at(stop);
	// работает быстро, но указатель может стать невалидным, если в каталог будет добавлена новая информация
}

int TransportCatalogue::getRealStopsDistance(const Stop* stopA, const Stop* stopB) const
{
	std::pair<const Stop*, const Stop*> pairAB{ stopA, stopB };
	if (stopsDistance.count(pairAB) > 0) {
		return stopsDistance.at(pairAB);
	}
	else {
		if (stopA == stopB) {
			return 0;
		}
		std::pair<const Stop*, const Stop*> pairBA{ stopB, stopA };
		if (stopsDistance.count(pairBA) > 0) {
			return stopsDistance.at(pairBA);
		}
	}
	return -1; // такого быть не должно!!!
}

std::vector<const Route*> transport_catalogue::TransportCatalogue::getAllRoutes() const
{
	std::vector<const Route*> routes;
	for (const std::pair<const std::string_view, Route*>& currentRoute : nameToRoute_) {
		routes.push_back(currentRoute.second);
	}
	return routes;
}
