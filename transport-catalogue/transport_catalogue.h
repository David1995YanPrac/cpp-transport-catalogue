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
        double curvature;
    };

    class TransportCatalogue
    {
    public:
        struct StopDistancesHasher 
        {
            size_t operator()(const std::pair<const Stop*, const Stop*>& points) const 
            {
                size_t hash_first = std::hash<const void*>{}(points.first);
                size_t hash_second = std::hash<const void*>{}(points.second);
                return hash_first + hash_second * 37;
            }
        };
        
        void AddRoute(const std::string& route_number, const std::vector<std::string>& route_stops, bool circular_route);
        void AddStop(const std::string& stop_name, geo::Coordinates& coordinates);

        const Bus* FindRoute(const std::string_view& route_number) const;
        const Stop* FindStop(const std::string_view& stop_name) const;

        const RouteInfo RouteInformation(const std::string& route_number) const;

        const std::unordered_set<std::string> GetBusesOnStop(const std::string& stop_name) const;

        void SetDistance(const Stop* from, const Stop* to, const int distance);
        int GetDistance(const Stop* from, const Stop* to) const;

    private:
        size_t UniqueStopsCount(const std::string& route_number) const;
        
        std::deque<Bus> all_buses_;
        std::deque<Stop> all_stops_;
        std::unordered_map<std::string, const Bus*> busname_to_bus_;
        std::unordered_map<std::string, const Stop*> stopname_to_stop_;

        std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistancesHasher> stop_distances_;
    };

}
