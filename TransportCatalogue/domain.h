#pragma once
#include <string_view>
#include <vector>
#include "geo.h"

namespace domain {
using namespace geo;

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
	int length;
	double curvature; //извилистость. отношение фактическиой длины маршрута к кратчайшей (географической) длине
};


} // namespace domain

//функция сравнивает два double. Здесь она потому что нужно, чтобы она была
bool IsZero(double value); 