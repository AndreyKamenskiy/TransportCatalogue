#pragma once
#include <optional>
#include <string_view>

#include "transport_catalogue.h"
#include "domain.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler {
public:
    using tcat = transport_catalogue::TransportCatalogue;

    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const  tcat& db/*, const renderer::MapRenderer& renderer*/);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<domain::Route>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
   // svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const tcat& db_;
   // const renderer::MapRenderer& renderer_;
};
