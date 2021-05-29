#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <cassert>
#include <iostream>
#include <vector>

//tests for TransportCatalogue class
void TCTest1() {
	using namespace std;
	TransportCatalogue tc;
	std::vector<std::string_view> stops{ "stop1"sv, "stop2"sv };

	tc.addStop(stops[0], {0.1, 0.2});
	tc.addStop(stops[1], { 0.2, 0.3});
	const Stop* stop = tc.findStop("stop1"s);
	assert(stop->name == "stop1"s);
	assert(stop->coordinates.lat == 0.1);
	assert(stop->coordinates.lng == 0.2);

	try {
		stop = tc.findStop("no such stop"s);
		assert(false);
	}
	catch (invalid_argument&) {
	
	}
	catch (exception&) {
		assert(false);
	}

	std::string busName = "bus1";
	tc.addRoute(busName, stops);
	const Route* rt = tc.findRoute("bus1");
	assert(rt->stops[0] == tc.findStop(stops[0]));

	try{
		stops.push_back("stop3"s);
		std::string busName2 = "bus2"s;
		tc.addRoute(busName2, stops);
		assert(false);
	}
	catch (std::invalid_argument&) {
		//std::cout << e.what() << std::endl;
	}
	catch (std::exception&) {
		assert(false);
	}

	rt = tc.findRoute("no such bus"s);
	assert(!rt);
}

void TCTest2() {
	using namespace std;
	TransportCatalogue tc;
	std::vector<std::string_view> stops{ "stop1"sv, "stop2"sv, "stop3"sv, "stop4"sv};
	std::vector<Coordinates> coord{ {0.0, 0.0}, {0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0} };
	std::vector<std::string> routeNames{ "route 1"s, "route2"s};
	for (size_t i = 0; i < stops.size(); i++)
	{
		tc.addStop(stops[i], coord[i]);
	}
	tc.addRoute(routeNames[0], stops);
	stops.resize(2);
	tc.addRoute(routeNames[1], stops);
	RouteInfo info1 = tc.getRouteInfo(routeNames[0]);
	assert(info1.stopsNumber == 4);
	assert(info1.uniqueStops == 4);
	double len = ComputeDistance(coord[0], coord[1]) + ComputeDistance(coord[2], coord[1]) + ComputeDistance(coord[2], coord[3]);
	assert(info1.length == len);
	RouteInfo info2 = tc.getRouteInfo("route2"s);
	assert(info2.stopsNumber == 2);
	assert(info2.uniqueStops == 2);
	len = ComputeDistance(coord[0], coord[1]);
	assert(info2.length == len);
}

void testTransportCatalogue() {
	TCTest1();
	TCTest2();

}

//tests for input_reader functions

TransportCatalogue makeSimpleCatalogue() {
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 10 << "\n";
	ss << "Stop Tolstopaltsevo: 55.611087, 37.208290"s << "\n";
	ss << "Stop Marushkino: 55.595884, 37.209755"s << "\n";
	ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << "\n";
	ss << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s << "\n";
	ss << "Stop Rasskazovka: 55.632761, 37.333324"s << "\n";
	ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"s << "\n";
	ss << "Stop Biryusinka: 55.581065, 37.648390"s << "\n";
	ss << "Stop Universam: 55.587655, 37.645687"s << "\n";
	ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << "\n";
	ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << "\n";
	TransportCatalogue tc;
	addToCatalogue(ss, tc);
	return tc;
}

void IRTest1() {
	using namespace std::string_literals;
	std::stringstream ss;
	ss << "NotANumber"s << "\n";
	TransportCatalogue tc;
	try {
		addToCatalogue(ss, tc);
		assert(false);
	}
	catch (std::invalid_argument&) {
		
	}
	catch (std::exception&) {
		assert(false);
	}
}

void IRTest2() {
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 10 << "\n";
	ss << "Stop Tolstopaltsevo: 55.611087, 37.208290"s << "\n";
	ss << "Stop Marushkino: 55.595884, 37.209755"s << "\n";
	ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << "\n";
	ss << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s << "\n";
	ss << "Stop Rasskazovka: 55.632761, 37.333324"s << "\n";
	ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"s << "\n";
	ss << "Stop Biryusinka: 55.581065, 37.648390"s << "\n";
	ss << "Stop Universam: 55.587655, 37.645687"s << "\n";
	ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << "\n";
	ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << "\n";
	TransportCatalogue tc;
	addToCatalogue(ss, tc);
	auto routeInfo = tc.getRouteInfo("256"s);
	assert(routeInfo.stopsNumber == 6);
	assert(routeInfo.uniqueStops == 5);
	assert(floor(routeInfo.length) == 4371.);
	routeInfo = tc.getRouteInfo("750"s);
	assert(routeInfo.stopsNumber == 5);
	assert(routeInfo.uniqueStops == 3);
	assert(floor(routeInfo.length) == 20939.);
	try {
		routeInfo = tc.getRouteInfo("noSuchRoute"s);
		assert(false);
	}
	catch (std::invalid_argument&) {
	
	}
	catch (std::exception&) {
		assert(false);
	}
}

void IRTest3() {
	TransportCatalogue tc = makeSimpleCatalogue();
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 3 << "\n";
	ss << "Bus 256"s << "\n";
	ss << "Bus 750"s << "\n";
	ss << "Bus 751"s << "\n";
	std::stringstream so;
	printQueries(ss, so, tc);
	const std::vector<std::string> answers = { "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length"s,
		"Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length"s,
		"Bus 751: not found"s
	};
	for (const std::string& ans : answers) {
		std::string line;
		std::getline(so, line);
		assert(line == ans);
	}
}



void IRTest4() {
	TransportCatalogue tc = makeSimpleCatalogue();
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 13 << "\n";
	ss << "Stop Tolstopaltsevo: 55.611087, 37.20829"s << "\n";
	ss << "Stop Marushkino: 55.595884, 37.209755"s << "\n";
	ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << "\n";
	ss << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s << "\n";
	ss << "Stop Rasskazovka: 55.632761, 37.333324"s << "\n";
	ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517"s << "\n";
	ss << "Stop Biryusinka: 55.581065, 37.64839"s << "\n";
	ss << "Stop Universam: 55.587655, 37.645687"s << "\n";
	ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << "\n";
	ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << "\n";
	ss << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << "\n";
	ss << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s << "\n";
	ss << "Stop Prazhskaya: 55.611678, 37.603831"s << "\n";
	ss << 6 << "\n";
	ss << "Bus 256"s << "\n";
	ss << "Bus 750"s << "\n";
	ss << "Bus 751"s << "\n";
	ss << "Stop Samara"s << "\n";
	ss << "Stop Prazhskaya"s << "\n";
	ss << "Stop Biryulyovo Zapadnoye"s << "\n";

	addToCatalogue(ss, tc);
	std::stringstream so;
	printQueries(ss, so, tc);
	const std::vector<std::string> answers = { "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length"s,
		"Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length"s,
		"Bus 751: not found"s,
		"Stop Samara: not found"s,
		"Stop Prazhskaya: no buses"s,
		"Stop Biryulyovo Zapadnoye: buses 256 828"s
	};
	for (const std::string& ans : answers) {
		std::string line;
		std::getline(so, line);
		assert(line == ans);
	}
}



void testInputReader() {
	IRTest1();
	IRTest2();
	IRTest3();
	IRTest4();
}


using namespace std;

int main() {
	testTransportCatalogue();
	testInputReader();

	cout << "all tests passed successfully"s;
	/*TransportCatalogue tc;
	addToCatalogue(cin, tc);
	printQueries(cin, cout, tc);*/

	return 0;
}