#include "input_reader.h"
#include "transport_catalogue.h"
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <iostream>

//TODO: Remove all string copyes
//TODO: add Check for invalid input like no stops on the route

enum class inputQueryType {
    ADD_STOP,
    ADD_ROUTE
};

inputQueryType getQueryType(std::string_view line) {
    using namespace std;
    const std::string_view addStopPrefix = "Stop "sv;
    const std::string_view addRoutePrefix = "Bus "sv;
    
    if ( line.substr(0, addStopPrefix.size() ) == addStopPrefix) {
        return inputQueryType::ADD_STOP;
    }
    if (line.substr(0, addRoutePrefix.size()) == addRoutePrefix) {
        return inputQueryType::ADD_ROUTE;
    }

    throw invalid_argument("Unknown input query: "s + static_cast<string>(line));
}

struct AddStopQuery{
    std::string_view name;
    Coordinates coordinates{0,0};
};

AddStopQuery parseAddStopQuery(std::string_view line) {
    using namespace std::string_literals;
    AddStopQuery res;
    auto pos = line.find(": "s, 5);
    res.name = line.substr(5, pos - 5);
    auto latEnd = line.find(", ", pos + 1);
    res.coordinates.lat = std::stod(static_cast<std::string>(line.substr(pos + 1, latEnd - (pos + 1))));
    res.coordinates.lng = std::stod(static_cast<std::string>(line.substr(latEnd + 1)));
    return res;
}

struct AddRouteQuery
{
    std::string_view name;
    std::vector<std::string_view> stopNames;
};

AddRouteQuery parseAddRouteQuery(std::string_view line) {
    using namespace std::string_literals;
    AddRouteQuery res;
    auto pos = line.find(": "s, 4);
    res.name = line.substr(4, pos - 4);
    auto stopBegin = pos + 2;
    std::string separator = " > ";
    auto stopEnd = line.find(separator, stopBegin);
    if (stopEnd == line.npos) {
        // it is a linear route with " - "s separator;
        separator[1] = '-';
        stopEnd  = line.find(separator, stopBegin);
    }
    while (true) {
        res.stopNames.push_back(line.substr(stopBegin, stopEnd - stopBegin));
        if (stopEnd == line.npos) {
            break;
        }
        stopBegin = stopEnd + 3;
        stopEnd = line.find(separator, stopBegin);
    }
    if (separator[1] == '-') {
        for (int i = res.stopNames.size() - 2; i >= 0; --i) {
            res.stopNames.push_back(res.stopNames[i]);
        }
    }
    return res;
}


std::istream& addToCatalogue(std::istream& input, TransportCatalogue& tc) {
    std::string queryCountStr;
    std::getline(input, queryCountStr);

    // может вызвать исключение std::invalid_argument и std::out_of_range. Пока никак не обрабатываем, пробрасываем дальше
    int queryCount = std::stoi(queryCountStr); 
    std::vector<AddRouteQuery> queries;
    std::vector<std::string> routeLines;

    for (size_t i = 0; i < queryCount; ++i)
    {
        std::string line;
        std::getline(input, line);
        inputQueryType type = getQueryType(line);
        if (type == inputQueryType::ADD_STOP) {
            AddStopQuery query = parseAddStopQuery(line);
            tc.addStop(query.name, query.coordinates);
        }
        else if (type == inputQueryType::ADD_ROUTE) {
            routeLines.push_back(move(line));
            queries.push_back(parseAddRouteQuery(routeLines.back()));
        }
    }

    //TODO: make special metodin TransportCatalogue
    for (auto& query : queries) {
        tc.addRoute(query.name, query.stopNames);
    }
    return input;
}