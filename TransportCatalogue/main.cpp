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
#include "map_renderer.h"

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
	tc.addRoute(busName, stops, true);
	const Route* rt = tc.findRoute("bus1");
	assert(rt->stops[0] == tc.findStop(stops[0]));

	try {
		stops.push_back("stop3"s);
		std::string busName2 = "bus2"s;
		tc.addRoute(busName2, stops, true);
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


void testDistance() {
	using namespace std;
	TransportCatalogue tc;

	string s1 = "stop1";
	string s2 = "stop2";
	string s3 = "stop3";
	string s4 = "stop4";
	tc.addStop(s1, { 43.587795, 39.716901 });
	tc.addStop(s2, { 43.58770, 39.71690 });
	tc.addStop(s3, { 43.58775, 39.71695 });
	tc.addStop(s4, { 43.58785, 39.71685 });
	const Stop* s1p = tc.findStop(s1);
	const Stop* s2p = tc.findStop(s2);
	const Stop* s3p = tc.findStop(s3);
	tc.addStopsDistance(s1p, s2p, 100);
	tc.addStopsDistance(s2p, s1p, 400);
	tc.addStopsDistance(s2p, s2p, 500);
	tc.addStopsDistance(s1p, s1p, 600);
	tc.addStopsDistance(s1p, s3p, 700);
	assert(tc.getRealStopsDistance(s1p, s2p) == 100);
	assert(tc.getRealStopsDistance(s2p, s1p) == 400);
	assert(tc.getRealStopsDistance(s2p, s2p) == 500);
	assert(tc.getRealStopsDistance(s1p, s1p) == 600);
	assert(tc.getRealStopsDistance(s1p, s3p) == 700);
	assert(tc.getRealStopsDistance(s3p, s1p) == 700);
	assert(tc.getRealStopsDistance(s3p, s3p) == 0);
	assert(tc.getRealStopsDistance(s3p, s2p) == -1);
	assert(tc.getRealStopsDistance(s2p, s3p) == -1);

}

void testTransportCatalogue() {
	TCTest1();
	testDistance();
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
	renderer::MapRenderer renderer(jr.get_render_options());
	RequestHandler rh(tc, renderer);
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
	TestCatalogueWithJsonFiles("input_test3.json", "output_test3.json");
}

void test_json_reader() {
	test_json_reader1();
}

bool operator==(const svg::Rgb& lhs, const svg::Rgb& rhs){
	return lhs.red == rhs.red && lhs.green == rhs.green && lhs.blue == rhs.blue;
}

bool operator==(const svg::Rgba& lhs, const svg::Rgba& rhs) {
	return lhs.red == rhs.red && lhs.green == rhs.green 
		&& lhs.blue == rhs.blue && IsZero(lhs.opacity - rhs.opacity);
}

void test_render_options() {
	using namespace std::literals::string_literals;
	std::string simpleTest = loadFile("s10_final_opentest_1.json"s);
	std::stringstream strm{ simpleTest };
	JsonReader jr{ strm };
	renderer::RenderOptions ops{ jr.get_render_options() };
	assert(IsZero( ops.width - 89298.28369209476));
	assert(IsZero(ops.height - 58011.29248202205));
	assert(IsZero(ops.padding - 22325.567226490493));
	assert(IsZero(ops.stop_radius - 21462.68635533567));
	assert(IsZero(ops.line_width - 38727.57356370373));
	assert(ops.stop_label_font_size == 86988);
	assert(IsZero(ops.stop_label_offset.x  + 23192.03299796056));
	assert(IsZero(ops.stop_label_offset.y - 92100.21839665441));
	assert(std::get<std::string>(ops.underlayer_color) == "coral"s);
	assert(IsZero(ops.underlayer_width - 34006.680510317055));
	assert(ops.bus_label_font_size == 78497);
	assert(IsZero(ops.bus_label_offset.x - 59718.916265509615));
	assert(IsZero(ops.bus_label_offset.y - 15913.541281271406));
	using namespace svg;

	assert(std::get<Rgba>(ops.color_palette[0]) == Rgba( 195, 60, 81, 0.6244132141059138 ));
	assert(std::get<Rgb>(ops.color_palette[1]) == Rgb ( 2, 81, 213 ));
	assert(std::get<Rgba>(ops.color_palette[2]) == Rgba( 81, 152, 19, 0.6834377639654173 ));
	assert(std::get<Rgba>(ops.color_palette[3]) == Rgba( 94, 70, 16, 0.7604371566734329 ));
	assert(std::get<Rgb>(ops.color_palette[4]) == Rgb( 191, 220, 104 ));
	assert(std::get<std::string>(ops.color_palette[5]) == "brown"s);
}

void test_layer_by_layer_render() {
	using namespace std::literals::string_literals;
	std::string simpleTest = loadFile("input_render_test1.json"s);
	std::stringstream strm{ simpleTest };
	JsonReader jr{ strm };
	TransportCatalogue tc;
	jr.add_to_catalogue(tc);
	renderer::MapRenderer renderer{ jr.get_render_options() };
	RequestHandler rh(tc, renderer);
	json::Document requestJSON = jr.get_responce(rh);
	auto answers = requestJSON.GetRoot().AsArray();
	for (auto answer : answers) {
		const auto& dict = answer.AsMap();
		if (dict.count("map"s) == 0) {
			continue;
		}
		std::string svgStr = dict.at("map"s).AsString();
		std::cout << svgStr << std::endl;

	}

}

void render_tests() {
	test_render_options();
	//test_layer_by_layer_render();
	TestCatalogueWithJsonFiles("s10_final_opentest_1.json", "s10_final_opentest_1_answer.json");
	//TestCatalogueWithJsonFiles("s10_final_opentest_2.json", "s10_final_opentest_2_answer.json");

	//TODO: написать качественный тест для svg
	TestCatalogueWithJsonFiles("s10_final_opentest_3.json", "s10_final_opentest_3_answer.json");
}

using namespace std;

int main() {

	/*testTransportCatalogue();
	setlocale(LC_CTYPE, "Russian");
	test_json_reader();
	render_tests();*/

	JsonReader jr{ cin };
	TransportCatalogue tc;
	jr.add_to_catalogue(tc);
	renderer::MapRenderer renderer(jr.get_render_options());
	RequestHandler rh(tc, renderer);
	json::Document requestJSON = jr.get_responce(rh);
	json::Print(requestJSON, std::cout);

	return 0;
}