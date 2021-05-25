#include "stat_reader.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

void printRouteInfo(std::ostream& output, std::string_view name, const RouteInfo& info) {
    using namespace std::string_literals;
    output << "Bus "s << name << ": "s << info.stopsNumber << " stops on route, "s << info.uniqueStops
        << " unique stops, "s << std::setprecision(6) << info.length << " route length\n"s;
}

void printQueries(std::istream& input, std::ostream& output, TransportCatalogue& tc) {
    std::string queryCountStr;
    std::getline(input, queryCountStr);
    // может вызвать исключение std::invalid_argument и std::out_of_range. Пока никак не обрабатываем, пробрасываем дальше
    int queryCount = std::stoi(queryCountStr);
    for (size_t i = 0; i < queryCount; ++i)
    {
        using namespace std;
        std::string line;
        std::getline(input, line);
        // since now we have just one type of the query check for it
        if (string_view( &line[0], 4 ) != "Bus "sv) {
            throw invalid_argument("not 'Bus X' query: "s + line);
        }
        string_view routeName(&line[0], line.size() - 4);
        RouteInfo info = tc.getRouteInfo(routeName);
        printRouteInfo(output, routeName, info);
    }
    output.flush();
}