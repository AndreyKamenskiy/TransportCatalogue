#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include "request_handler.h"
#include "map_renderer.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
	: db_(db),  renderer_(renderer)
{
}

std::optional<domain::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	try {
		return db_.getRouteInfo(bus_name);
	}
	catch (std::invalid_argument&) {
		return {};
	}
}

const std::unordered_set<domain::Route*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	return  db_.getRoutesOnStop(db_.findStop(stop_name));
}

svg::Document RequestHandler::RenderMap() const
{
	using namespace domain;
	// ����� ��� ���������� ���������, ������� ����� �������� �� �����
	std::vector<const Route*> routesToRender;
	std::vector<const Stop*> stopsToRender;
	{	// fill routesToRender, stopsToRender
		const std::vector<const Route*> routes = std::move(db_.getAllRoutes());
		routesToRender.reserve(routes.size());
		std::unordered_set<const Stop*> allStops;
		std::vector<domain::Coordinates> points;
		for (const auto* route : routes) {
			if (!route->stops.empty()) {
				routesToRender.push_back(route);
				for (const Stop* stop : route->stops) {
					if (allStops.count(stop) == 0) {
						allStops.insert(stop);
						stopsToRender.push_back(stop);
						points.push_back(stop->coordinates);
					}
				}
			}
		}

		// ������ ������������ ��������������.
		renderer_.InitProjector(points);
	}

	svg::Document doc;
	//������������� �������� � ���������
	std::sort(routesToRender.begin(), routesToRender.end(), 
		[](const Route* a, const Route* b) { return a->name < b->name; });
	std::sort(stopsToRender.begin(), stopsToRender.end(),
		[](const Stop* a, const Stop* b) { return a->name < b->name; });

	// 1� ����. �������� ����� ���������.
	size_t count = 0;
	for (const Route* route : routesToRender) {
		//��� ������� ���������������� ��������
		// ������� �������� ����� renderer_ add_route_line
		renderer_.RenderRoute(doc, route, count++);
	}
	// 2� ����. �������� ���������.
	count = 0;
	for (const Route* route : routesToRender) {
		//��� ������� ���������������� ��������
		// ������� �������� ��������, ���� �� ���������, �� ������� 2 ��������.
		renderer_.RenderRouteLables(doc, route, count++);
	}

	// 3� ����. ������� ���������.
	for (const Stop* stop : stopsToRender) {
		// ������� �������� ��������� �� �������������� �������
		renderer_.RenderStopPoint(doc, stop);
	}
	// 4� ����. �������� ���������.
		// ������� �������� ��������� �� �������������� ����


	return doc;
}



