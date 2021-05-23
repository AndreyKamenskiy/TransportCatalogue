#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <vector>

#include "geo.h"


//Структура, описывающая остановку
struct Stop {
	std::string_view name;
	Coordinates coordinates;
};

//Типы маршрутов 
enum class RouteType {
	LINEAR,
	CIRCLE
};

//Структура, описывающая маршрут
struct Route {
	std::string_view name;
	RouteType type;
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

	TransportCatalogue() {
		//default constructor
	}

	// добавление маршрута в базу
	void addRoute(std::string& name, RouteType type, std::vector<std::string>& stops);

	//добавление остановки в базу
	void addStop(std::string& name, Coordinates coordinates);

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
	std::unordered_map<std::string_view, Stop*> nameToStop_;
	std::unordered_map<std::string_view, Route*> nameToRoute_;
	std::unordered_map<const Stop*, std::vector<const Route*>> stopToRoutes_;

	//список всех остановок
	std::list<Stop> stops_;

	//список всех маршрутов
	std::list<Route> routes_;

	std::string_view addString(std::string& str) {
		allNames.push_back(str);
		return allNames.back();
	}

};