#include <stdexcept>
#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db)
	: db_(db)/*,  renderer_(renderer)*/
{
}

std::optional<domain::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	try {
		return db_.getRouteInfo(bus_name);
	}
	catch (std::invalid_argument& exc) {
		return {};
	}
}

const std::unordered_set<domain::Route*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	return  db_.getRoutesOnStop(db_.findStop(stop_name));
}



