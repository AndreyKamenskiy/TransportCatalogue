#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "stat_reader.h"

using namespace transport_catalogue;

enum class outputQueryType {
    BUS,
    STOP
};


outputQueryType getStatQueryType(std::string_view line) {
    using namespace std;
    const std::string_view showStopPrefix = "Stop "sv;
    const std::string_view showRoutePrefix = "Bus "sv;

    if (line.substr(0, showStopPrefix.size()) == showStopPrefix) {
        return outputQueryType::STOP;
    }
    if (line.substr(0, showRoutePrefix.size()) == showRoutePrefix) {
        return outputQueryType::BUS;
    }

    throw invalid_argument("Unknown ouput query: "s + static_cast<string>(line));
}


void printRouteInfo(std::ostream& output, std::string_view name, const RouteInfo& info) {
    using namespace std::string_literals;
    output << "Bus "s << name << ": "s << info.stopsNumber << " stops on route, "s << info.uniqueStops
        << " unique stops, "s << info.length << " route length, "s << std::setprecision(6) << info.curvature << " curvature\n"s;
}

void printNoRouteFound(std::ostream& output, std::string_view name) {
    using namespace std::string_literals;
    output << "Bus "s << name << ": not found\n"s;
}

void manageBusQuery(std::ostream& output, std::string& line, TransportCatalogue& tc) {
    using namespace std;
    string_view routeName(&line[4], line.size() - 4);
    try {
        RouteInfo info = tc.getRouteInfo(routeName);
        printRouteInfo(output, routeName, info);
    }
    catch (invalid_argument&) {
        printNoRouteFound(output, routeName);
    }

}


void printRoutesOnStop(std::ostream& output, const Stop* stop, TransportCatalogue& tc) {
    using namespace std::string_literals;
    output << "Stop "s << stop->name << ": "s;
    if (tc.getRoutesNumOnStop(stop) == 0) {
        output << "no buses\n"s;
        return;
    }
    const std::unordered_set<domain::Route*>* routesSet = tc.getRoutesOnStop(stop);
    std::vector<Route*> routes{routesSet->cbegin(), routesSet->cend()};
    std::sort(routes.begin(), routes.end(), [](const Route* a, const Route* b) { return a->name < b->name; });
    output << "buses"s;
    for (const Route* route : routes) {
        output << ' ' << route->name;
    }
    output << '\n';
}

void printNoStopFound(std::ostream& output, std::string_view name) {
    //TODO: combine methods printNoStopFound and printNoStopFound
    using namespace std::string_literals;
    output << "Stop "s << name << ": not found\n"s;
}


void manageStopQuery(std::ostream& output, std::string& line, TransportCatalogue& tc) {
    using namespace std;
    string_view stopName(&line[5], line.size() - 5);
    try {
        const Stop* stop = tc.findStop(stopName);
        printRoutesOnStop(output, stop, tc);
    }
    catch (invalid_argument&) {
        printNoStopFound(output, stopName);
    }
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
        outputQueryType type = getStatQueryType(line);

        if (type == outputQueryType::BUS) {
            manageBusQuery(output, line, tc);
            continue;
        }

        if (type == outputQueryType::STOP) {
            manageStopQuery(output, line, tc);
            continue;
        }
        throw invalid_argument("uncknown query type: "s + line);
    }
    output.flush();
}