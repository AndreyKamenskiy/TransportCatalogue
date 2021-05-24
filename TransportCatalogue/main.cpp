#include "transport_catalogue.h"
#include "input_reader.h"
#include <cassert>
#include <iostream>

//tests for TransportCatalogue class
void TCTest1() {
	using namespace std::string_literals;
	TransportCatalogue tc;
	std::vector<std::string> stops{ "stop1"s, "stop2"s };

	tc.addStop(stops[0], {0.1, 0.2});
	tc.addStop(stops[1], { 0.2, 0.3});
	const Stop* stop = tc.findStop("stop1"s);
	assert(stop->name == "stop1"s);
	assert(stop->coordinates.lat == 0.1);
	assert(stop->coordinates.lng == 0.2);

	stop = tc.findStop("no such stop"s);
	assert(!stop);

	std::string busName = "bus1";
	tc.addRoute(busName, RouteType::LINEAR, stops);
	const Route* rt = tc.findRoute("bus1");
	assert(rt->type == RouteType::LINEAR);
	assert(rt->stops[0] == tc.findStop(stops[0]));

	try{
		stops.push_back("stop3"s);
		std::string busName2 = "bus2"s;
		tc.addRoute(busName2, RouteType::LINEAR, stops);
		assert(false);
	}
	catch (std::invalid_argument& e) {
		std::cout << e.what() << std::endl;
	}
	catch (std::exception& e) {
		assert(false);
	}

	rt = tc.findRoute("no such bus"s);
	assert(!rt);
}

void TCTest2() {
	using namespace std::string_literals;
	TransportCatalogue tc;
	std::vector<std::string> stops{ "stop1"s, "stop2"s, "stop3"s, "stop4"s};
	std::vector<Coordinates> coord{ {0.0, 0.0}, {0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0} };
	std::vector<std::string> routeNames{ "route 1"s, "route2"s};
	for (size_t i = 0; i < stops.size(); i++)
	{
		tc.addStop(stops[i], coord[i]);
	}
	tc.addRoute(routeNames[0], RouteType::LINEAR, stops);
	stops.resize(2);
	tc.addRoute(routeNames[1], RouteType::CIRCLE, stops);
	RouteInfo info1 = tc.getRouteInfo(routeNames[0]);
	assert(info1.stopsNumber == 4);
	assert(info1.uniqueStops == 2);
	double len = ComputeDistance(coord[0], coord[1]) + ComputeDistance(coord[2], coord[1]) + ComputeDistance(coord[2], coord[3]);
	assert(info1.length == len);
	RouteInfo info2 = tc.getRouteInfo("route2"s);
	assert(info2.stopsNumber == 2);
	assert(info2.uniqueStops == 0);
	len = ComputeDistance(coord[0], coord[1]) * 2;
	assert(info2.length == len);
}

void testTransportCatalogue() {
	TCTest1();
	TCTest2();

}

//tests for input_reader functions

void IRTest1() {
	using namespace std::string_literals;
	std::stringstream ss;
	ss << "NotANumber"s << "\n";
	TransportCatalogue tc;
	try {
		addToCatalogue(ss, tc);
		assert(false);
	}
	catch (std::invalid_argument& e) {
		
	}
	catch (std::exception& e) {
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
	ss << "Stop Biryulyovo Zapadnoye : 55.574371, 37.651700"s << "\n";
	ss << "Stop Biryusinka: 55.581065, 37.648390"s << "\n";
	ss << "Stop Universam: 55.587655, 37.645687"s << "\n";
	ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << "\n";
	ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << "\n";
	TransportCatalogue tc;
	addToCatalogue(ss, tc);
}


void testInputReader() {
	IRTest1();
	IRTest2();



		
		
		
		
		

}


using namespace std;

int main() {
	testTransportCatalogue();
	testInputReader();

	cout << "all tests passed successfully"s;

	return 0;
}