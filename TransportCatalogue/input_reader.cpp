#include "input_reader.h"
#include "transport_catalogue.h"
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <iostream>

//TODO: add Check for invalid input like no stops on the route

enum class inputQueryType {
    ADD_STOP,
    ADD_ROUTE
};

inputQueryType getInputQueryType(std::string_view line) {
    using namespace std;
    const std::string_view addStopPrefix = "Stop "sv;
    const std::string_view addRoutePrefix = "Bus "sv;

    if (line.substr(0, addStopPrefix.size()) == addStopPrefix) {
        return inputQueryType::ADD_STOP;
    }
    if (line.substr(0, addRoutePrefix.size()) == addRoutePrefix) {
        return inputQueryType::ADD_ROUTE;
    }

    throw invalid_argument("Unknown input query: "s + static_cast<string>(line));
}

struct AddStopQuery {
    std::string_view name;
    Coordinates coordinates{ 0,0 };
    //имена других остановок и дистанции до них.
    std::vector<std::pair<std::string_view, double>> distances;
};

AddStopQuery parseAddStopQuery(std::string_view line) {
    using namespace std::string_literals;
    AddStopQuery res;
    //Имя остановки заканчивается на ':'. 
    auto pos = line.find(": "s, 5);
    //Имя остановки начинается с пятого символа запроса "Stop NAME: "
    res.name = line.substr(5, pos - 5);
    //ищем ", " после двоеточия. Это конец широты.
    auto latEnd = line.find(", ", pos + 2);
    auto lngEnd = line.find(", ", latEnd + 2);
    res.coordinates.lat = std::stod(static_cast<std::string>(line.substr(pos + 2, latEnd - (pos + 2))));
    res.coordinates.lng = std::stod(static_cast<std::string>(line.substr(latEnd + 2, lngEnd - (latEnd + 2))));
    auto separatorBegin = lngEnd;
    //пока помимо координат есть дистанции до других остановок
    while (separatorBegin != line.npos) {
        auto sublineEnd = line.find(", ", separatorBegin + 2);;
        //обрабатываем подстроку с дистанцией до другой остановки
        // формат "D1m to stop1, "
        std::pair<std::string_view, double> dictanceToStop;
        auto distEnd = line.find("m to ", separatorBegin + 2);
        if (distEnd == line.npos) {
            using namespace std;
            throw invalid_argument("Unknown input query type: "s + static_cast<string>(line));
        }
        //парсим дистанцию
        dictanceToStop.second = std::stod(static_cast<std::string>(line.substr(separatorBegin + 2, distEnd - (separatorBegin + 2))));
        //парсим имя остановки
        dictanceToStop.first = line.substr(distEnd + 5, sublineEnd - (distEnd + 5));
        res.distances.push_back(dictanceToStop);
        separatorBegin = sublineEnd;
        sublineEnd = line.find(", ", separatorBegin + 2);
    }
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
        stopEnd = line.find(separator, stopBegin);
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
        inputQueryType type = getInputQueryType(line);
        if (type == inputQueryType::ADD_STOP) {
            AddStopQuery query = parseAddStopQuery(line);
            if (tc.hasStop(query.name)) {
                //если такая остановка встречалась ранее, то обновим координаты
                tc.updateStopCoordinates(query.name, query.coordinates);
            }
            else {
                tc.addStop(query.name, query.coordinates);
            }
            // добавим дистанции до других остановок.
            const Stop* stop = tc.findStop(query.name);
            for (auto [toStopName, distance] : query.distances) {
                const Stop* toStop;
                if (tc.hasStop(toStopName)) {
                    toStop = tc.findStop(toStopName);
                }
                else {
                    // добавим остановку с нулевыми координатами.
                    //TODO: addStop must return pointer to Stop.
                    tc.addStop(toStopName);
                    toStop = tc.findStop(toStopName);
                }
                // добавим дистанцию до остановки
                tc.addStopsDistance(stop, toStop, distance);
            }
        }
        else if (type == inputQueryType::ADD_ROUTE) {

            //TODO: inline add new route. if new stop - > genegate new Stop. routeLen calc after all Line parsing
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