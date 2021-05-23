#include "transport_catalogue.h"
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

void testTransportCatalogue() {
	TCTest1();

}

using namespace std;

int main() {
	testTransportCatalogue();
	cout << "all tests passed successfully"s;

	return 0;
}