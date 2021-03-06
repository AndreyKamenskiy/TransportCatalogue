#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"


namespace transport_catalogue {
	//???????????? ???? ???????? ??? ????, ????? ?????? ???? ???? ????? ???????? domain ? geo
	using namespace domain;

// ????? ????????????? ??????????? 
class TransportCatalogue {

public:

	// ?????????? ???????? ? ????
	void addRoute(std::string_view name, std::vector<std::string_view>& stops, bool isCircle);

	//?????????? ????????? ? ????
	void addStop(std::string_view name, Coordinates coordinates);

	//???????? ????????? ??? ?????????. ?????????? ??????????? ????? ????? updateCoordinates;
	//????? ??? ????, ????? ???????? ????????? ?????, ????? ??? ??????????? ??????? ???.
	void addStop(std::string_view name);

	// ???????? ?????????? ?????????. ??????? ??? ????, ????? ??? ????????? ?????????? ? ??????????? 
	// ????????? ???????? ??.
	void updateStopCoordinates(std::string_view name, Coordinates coordinates);

	//????? ???????? ?? ?????
	const Route* findRoute(std::string_view name)const;

	//????? ????????? ?? ?????
	const Stop* findStop(std::string_view name) const;

	// ???????? ???? ?? ????? ??????????
	bool hasStop(std::string_view name) const;

	//????????? ????????? ????? ????? ???????????
	void addStopsDistance(const Stop* stopA, const Stop* stopB, int distance);

	//????????? ?????????? ? ????????
	const RouteInfo getRouteInfo(const Route* route) const;
	const RouteInfo getRouteInfo(const std::string_view& routeName) const;

	//????? ????????? ?? ????????? 
	size_t getRoutesNumOnStop(const Stop* stop) const;

	const std::unordered_set<domain::Route*>* getRoutesOnStop(const Stop* stop) const;

	// ???????? ?????????? ????? ???????????. ?????????? ???????? ?????????? ????? ???????????.
	//???? ?????????? ?? ???????? ?????????? -1. ;
	//???? ?????????? ?? ? ?? ? ?? ??????, ?????????? 0. ;
	int getRealStopsDistance(const Stop* stopA, const Stop* stopB) const;

	//???????? ?????? ?? ??? ????????
	std::vector<const Route*> getAllRoutes() const;
		
	// ?? ?????? ?????? ??? ????????? ???? ????? ???? ???? ???????
	~TransportCatalogue() = default;

private:

	//?? ???????????? ??????????. ????? ??? ???????? ??????? ????????? ? ?????????.
	const Coordinates voidCoordinates = { 200.0, 200.0 };

	// ????????? ??? ???? ???? ???????????. ?? ??? ????? ????????? string_view ??????.
	std::list<std::string> allNames;
	std::unordered_map<std::string_view, Stop*> nameToStop_; // key - name of the stop, value - ptr to Stop
	std::unordered_map<std::string_view, Route*> nameToRoute_;

	//?????? ???? ?????????
	std::list<Stop> stops_;

	//?????? ???? ?????????
	std::list<Route> routes_;

	// ???????. ????????? - ????????
	std::unordered_map<const Stop*, std::unordered_set<Route*>> stopToRoutes_;


	// ????? ???? ?????????? ?? ?????????. ???? ??????? ????? A + ? ? ? + ?.
	class TwoStopHasher {
	public:
		size_t operator()(const std::pair<const Stop*, const Stop*> stops) const {
			std::hash<const void*> phasher; // ????? ??? ?????????.
			size_t a = phasher(stops.first);
			size_t b = phasher(stops.second);
			return (a + b) * (a + b + 1) / 2 + b ;
		}
	};

	// ???????. ?????????? ????? ???????????. ?????????? ?? ? ?? ? ????? ???? ?? ????? ?????????? ?? ? ?? ?.
	// ????? ????? ????????? ?????????? ?? ? ?? ?.
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, TwoStopHasher> stopsDistance;

	std::string_view addString(std::string_view str) {
		allNames.push_back(static_cast<std::string>(str));
		return allNames.back();
	}

};

}