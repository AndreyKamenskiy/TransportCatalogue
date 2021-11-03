#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <locale.h>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace transport_catalogue;
using namespace domain;

//tests for TransportCatalogue class
void TCTest1() {
	using namespace std;
	TransportCatalogue tc;
	std::vector<std::string_view> stops{ "stop1"sv, "stop2"sv };

	tc.addStop(stops[0], { 0.1, 0.2 });
	tc.addStop(stops[1], { 0.2, 0.3 });
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

	try {
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
	std::vector<std::string_view> stops{ "stop1"sv, "stop2"sv, "stop3"sv, "stop4"sv };
	std::vector<Coordinates> coord{ {0.0, 0.0}, {0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0} };
	std::vector<std::string> routeNames{ "route 1"s, "route2"s };
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
	//TCTest2();

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

void IRTest5() {
	TransportCatalogue tc;
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 3 << "\n";
	ss << "Stop s1: 1.1, 1.1, 9900m to s2, 1000m to s3, 150m to s1"s << "\n";
	ss << "Stop s2: 1.2, 1.2, 10500m to s1, 1500m to s3, 10m to s2"s << "\n";
	ss << "Stop s3: 1.3, 1.3"s << "\n";
	ss << 0 << "\n";
	addToCatalogue(ss, tc);
	const Stop* s1 = tc.findStop("s1"s);
	const Stop* s2 = tc.findStop("s2"s);
	const Stop* s3 = tc.findStop("s3"s);
	assert(tc.getRealStopsDistance(s1, s1) == 150.0);
	assert(tc.getRealStopsDistance(s1, s2) == 9900.0);
	assert(tc.getRealStopsDistance(s1, s3) == 1000.0);
	assert(tc.getRealStopsDistance(s2, s1) == 10500.0);
	assert(tc.getRealStopsDistance(s2, s2) == 10.0);
	assert(tc.getRealStopsDistance(s2, s3) == 1500.0);
	assert(tc.getRealStopsDistance(s3, s1) == 1000.0);
	assert(tc.getRealStopsDistance(s3, s2) == 1500.0);
	assert(tc.getRealStopsDistance(s3, s3) == 0.0);
}

void IRTest6() {
	TransportCatalogue tc;
	using namespace std::string_literals;
	std::stringstream ss;
	ss << 13 << "\n";
	ss << "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s << "\n";
	ss << "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s << "\n";
	ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << "\n";
	ss << "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"s << "\n";
	ss << "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s << "\n";
	ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"s << "\n";
	ss << "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam"s << "\n";
	ss << "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya"s << "\n";
	ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya"s << "\n";
	ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye"s << "\n";
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
	const std::vector<std::string> answers = {
		"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature"s,
		"Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature"s,
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
	//IRTest2();
	//IRTest3();
	//IRTest4();
	IRTest5();
	IRTest6();
}


std::string loadFile(std::string filename) {
	std::ifstream is(filename, std::ios::binary | std::ios::ate);
	if (!is.is_open()) {
		return "";
	}
	auto size = is.tellg();
	std::string str(size, '\0'); // construct string to stream size
	is.seekg(0);
	if (!is.read(&str[0], size)) {
		str = "";
	}
	is.close();
	return str;
}

void test_json_reader1() {
	using namespace std::literals::string_literals;
	std::string simpleTest = loadFile("input_test1.json");
	std::stringstream strm{simpleTest};
	/*json::Document inputJSON = json::Load(strm);
	json::Print(inputJSON, std::cout);
	std::cout << "\n";*/
	JsonReader jr{strm};
	TransportCatalogue tc;
	jr.add_to_catalogue(tc);
	assert(tc.hasStop("Ривьерский мост"s));
	assert(tc.hasStop("Морской вокзал"s));
	
	RequestHandler rh(tc);
	json::Document requestJSON = jr.get_responce(rh);
	json::Print(requestJSON, std::cout);

	std::string simpleResponse = loadFile("output_test1.json");
	std::stringstream str1{ simpleResponse };
	json::Document outJSON = json::Load(str1);
	json::Print(outJSON, std::cout);

	assert(requestJSON == outJSON);
}

void test_json_reader() {
	test_json_reader1();

}



using namespace std;

int main() {

	testTransportCatalogue();
	testInputReader();
	setlocale(LC_CTYPE, "Russian");
	test_json_reader();
	//todo: добавить тест на некорректные запросы. нет полей, поля неправильного типа и т.п.

	cout << "all tests passed successfully"s;

	/*TransportCatalogue tc;
	addToCatalogue(cin, tc);
	printQueries(cin, cout, tc);*/

	return 0;
}