#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <locale.h>
#include <sstream>
#include "transport_catalogue.h"
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

void testTransportCatalogue() {
	TCTest1();
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

void TestCatalogueWithJsonFiles(std::string inputFile, std::string answerFile) {
	using namespace std::literals::string_literals;
	std::string simpleTest = loadFile(inputFile);
	std::stringstream strm{ simpleTest };
	JsonReader jr{ strm };
	TransportCatalogue tc;
	jr.add_to_catalogue(tc);
	RequestHandler rh(tc);
	json::Document requestJSON = jr.get_responce(rh);
	std::string simpleResponse = loadFile(answerFile);
	std::stringstream str1{ simpleResponse };
	json::Document outJSON = json::Load(str1);
	//json::Print(requestJSON, std::cout);
	//json::Print(outJSON, std::cout);
	assert(requestJSON == outJSON);
}


void test_json_reader1() {
	TestCatalogueWithJsonFiles("input_test1.json", "output_test1.json");
	TestCatalogueWithJsonFiles("input_test2.json", "output_test2.json");
}

void test_json_reader() {
	test_json_reader1();
}

using namespace std;

int main() {

	/*testTransportCatalogue();
	setlocale(LC_CTYPE, "Russian");
	test_json_reader();
	//todo: добавить тест на некорректные запросы. нет полей, поля неправильного типа и т.п.

	//cout << "all tests passed successfully"s; */

	
	JsonReader jr{cin};
	TransportCatalogue tc;
	jr.add_to_catalogue(tc);
	RequestHandler rh(tc);
	json::Document requestJSON = jr.get_responce(rh);
	json::Print(requestJSON, cout);
	

	return 0;
}