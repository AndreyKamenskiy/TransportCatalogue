#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"


//Структура, описывающая остановку
struct Stop {
	std::string_view name;
	Coordinates coordinates;
};

//Структура, описывающая маршрут
struct Route {
	std::string_view name;
	std::vector<const Stop*> stops;
};

//Статистическая информация о маршруте
struct RouteInfo {
	int stopsNumber;
	int uniqueStops;
	double length;
};

// Класс транспортного справочника 
class TransportCatalogue {

public:

	// добавление маршрута в базу
	void addRoute(std::string_view name, std::vector<std::string_view>& stops);
	//TODO: add input as AddRouteQuery

	//добавление остановки в базу
	void addStop(std::string_view name, Coordinates coordinates);

	//поиск маршрута по имени
	const Route* findRoute(std::string_view name);

	//поиск остановки по имени
	const Stop* findStop(std::string_view name);
	 
	//получение информации о маршруте
	const RouteInfo getRouteInfo(const Route* route);
	const RouteInfo getRouteInfo(const std::string_view routeName);

private:
	// контейнер для всех имен спарвочника. На них будут ссылаться string_view класса.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_; // key - name of the stop, value - ptr to Stop
	std::unordered_map<std::string_view, Route*> nameToRoute_;

	//список всех остановок
	std::list<Stop> stops_;

	//список всех маршрутов
	std::list<Route> routes_;

	std::string_view addString(std::string_view str) {
		allNames.push_back(static_cast<std::string>(str));
		return allNames.back();
	}

};