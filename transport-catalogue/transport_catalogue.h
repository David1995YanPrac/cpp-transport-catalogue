#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <string_view>
#include <set>

namespace transport_catalogue
{
    struct Bus
    {
        std::string number;
        std::optional<std::vector<std::string>> stops;
        bool circular_route;
    };

    struct Stop
    {
        std::string name;
        geo::Coordinates coordinates;
        std::unordered_set<std::string> buses;
    };

    struct RouteInfo
    {
        size_t stops_count;
        size_t unique_stops_count;
        double route_length;
    };

    class TransportCatalogue
    {
    public:
        void AddRoute(const std::string& route_number, const std::vector<std::string>& route_stops, bool circular_route);
        void AddStop(const std::string& stop_name, geo::Coordinates& coordinates);

        const Bus* FindRoute(const std::string_view& route_number) const;
        const Stop* FindStop(const std::string_view& stop_name) const;

        const RouteInfo RouteInformation(const std::string& route_number) const;

        //не могу его убрать в приватную область, так как я использую этот метод напрямую в stat_reader.cpp. Возможно я Вас не правильно понял))
        const std::unordered_set<std::string> GetBusesOnStop(const std::string& stop_name) const;

    private:
        size_t UniqueStopsCount(const std::string& route_number) const;
        
        std::deque<Bus> all_buses_;
        std::deque<Stop> all_stops_;
        std::unordered_map<std::string, const Bus*> busname_to_bus_;
        std::unordered_map<std::string, const Stop*> stopname_to_stop_;
    };

}
