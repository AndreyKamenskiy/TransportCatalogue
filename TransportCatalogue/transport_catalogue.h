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
	double curvature; //извилистость. отношение фактическиой длины маршрута к кратчайшей (географической) длине
};

// Класс транспортного справочника 
class TransportCatalogue {

public:

	// добавление маршрута в базу
	void addRoute(std::string_view name, std::vector<std::string_view>& stops);
	//TODO: add input as AddRouteQuery

	//добавление остановки в базу
	void addStop(std::string_view name, Coordinates coordinates);

	//добавить остановку буз координат. Координаты добавляются позже чкрез updateCoordinates;
	//Нужно для того, чтобы добавить остановку тогда, когда она встретилась впервый раз.
	void addStop(std::string_view name);

	// обновить координаты остановки. Сделано для того, чтобы при появлении информации о координатах 
	// остановки добавить их.
	void updateStopCoordinates(std::string_view name, Coordinates coordinates);

	//поиск маршрута по имени
	const Route* findRoute(std::string_view name);

	//поиск остановки по имени
	const Stop* findStop(std::string_view name);

	// проверка есть ли такая остановка?
	bool hasStop(std::string_view name);

	//сохранить дистанцию между двумя остановками
	void addStopsDistance(const Stop* stopA, const Stop* stopB, double distance);

	//получение информации о маршруте
	const RouteInfo getRouteInfo(const Route* route);
	const RouteInfo getRouteInfo(const std::string_view routeName);

	//Поиск маршрутов по остановке 
	size_t getRoutesNumOnStop(const Stop* stop);

	std::vector<const Route*> getRoutesOnStop(const Stop* stop);

	// Получить расстояние между остановками. Возвращает реальное расстояние между остановками.
	//если расстояние не известно возвращает -1. ;
	//если расстояние от А до А не задано, возвращает 0. ;
	double getRealStopsDistance(const Stop* stopA, const Stop* stopB);

private:

	//не существующие координаты. нужны для проверки наличия координат у остановки.
	const Coordinates voidCoordinates = { 200.0, 200.0 };

	// контейнер для всех имен спарвочника. На них будут ссылаться string_view класса.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_; // key - name of the stop, value - ptr to Stop
	std::unordered_map<std::string_view, Route*> nameToRoute_;

	//список всех остановок
	std::list<Stop> stops_;

	//список всех маршрутов
	std::list<Route> routes_;

	// словарь. остановка - маршруты
	std::unordered_map<const Stop*, std::unordered_set<Route*>> stopToRoutes_;


	// хэшер двух указателей на остановки. есть разница между A + Б и Б + А.
	class TwoStopHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*> stops) const {
			/*TODO: modify like here https ://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
			hash(a, b) = (a + b) * (a + b + 1 ) / 2 + b; */

			std::hash<const void*> phasher; // хэшер для указателя.
			return phasher(stops.first) + phasher(stops.second);
		}
	};

	// словарь. расстояние между остановками. Расстояние от А до Б может быть не равно расстоянию от Б до А.
	// Также может храниться расстояние от А до А.
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, TwoStopHasher> stopsDistance;

	std::string_view addString(std::string_view str) {
		allNames.push_back(static_cast<std::string>(str));
		return allNames.back();
	}

};