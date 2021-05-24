#include "input_reader.h"
#include <string>
#include <string_view>
#include <stdexcept>

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
    std::string name;
    Coordinates coordinates;
};

AddStopQuery parseAddStopQuery(std::string_view line) {
    using namespace std::string_literals;
    AddStopQuery res;
    auto pos = line.find(": "s, 4);
    res.name = static_cast<std::string>(line.substr(4, pos - 4));
    auto latEnd = line.find(", ", pos + 1);
    res.coordinates.lat = std::stod(static_cast<std::string>(line.substr(pos + 1, latEnd - (pos + 1))));
    res.coordinates.lng = std::stod(static_cast<std::string>(line.substr(latEnd + 1)));
    return res;
}




std::istream& addToCatalogue(std::istream& input, TransportCatalogue& tc) {
    std::string queryCountStr;
    std::getline(input, queryCountStr);

    // может вызвать исключение std::invalid_argument и std::out_of_range. Пока никак не обрабатываем, пробрасываем дальше
    int queryCount = std::stoi(queryCountStr); 
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
            
        }
    }
    return input;
}