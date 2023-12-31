#include "transport_catalogue.h"

namespace transport_catalogue
{
    void TransportCatalogue::AddRoute(const std::string& route_number, const std::vector<std::string>& route_stops, bool circular_route)
    {
        all_buses_.push_back({ route_number, route_stops, circular_route });

        for (const auto& route_stop : route_stops)
        {
            for (auto& stop_ : all_stops_)
            {
                if (stop_.name == route_stop) stop_.buses.insert(route_number);
            }
        }
        busname_to_bus_[all_buses_.back().number] = &all_buses_.back();
    }

    void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates& coordinates)
    {
        all_stops_.push_back({ stop_name, coordinates, {} });
        stopname_to_stop_[all_stops_.back().name] = &all_stops_.back();
    }

    const Bus* TransportCatalogue::FindRoute(const std::string_view& route_number) const
    {
        std::string route(route_number);
        return busname_to_bus_.count(route) ? busname_to_bus_.at(route) : nullptr;
    }

    const Stop* TransportCatalogue::FindStop(const std::string_view& stop_name) const
    {
        std::string stop(stop_name);
        return stopname_to_stop_.count(stop) ? stopname_to_stop_.at(stop) : nullptr;
    }

    const RouteInfo TransportCatalogue::RouteInformation(const std::string& route_number) const
    {
        RouteInfo route_info{};
        const Bus* bus = FindRoute(route_number);

        if (!bus) throw std::invalid_argument("bus not found");
        if (bus->circular_route) route_info.stops_count = bus->stops->size();
        else route_info.stops_count = bus->stops->size() * 2 - 1;

        double route_length_geograf = 0.0;
        double route_length = 0.0;
        for (auto iter = bus->stops.value().begin(); iter + 1 != bus->stops.value().end(); ++iter)
        {
            auto from = stopname_to_stop_.find(*iter)->second;
            auto to = stopname_to_stop_.find(*(iter + 1))->second;

            if (bus->circular_route)
            {
                route_length += GetDistance(from, to);
                
                route_length_geograf += ComputeDistance(from->coordinates, to->coordinates);
            }
            else
            {
                route_length += GetDistance(from, to) + GetDistance(to, from);

                route_length_geograf += ComputeDistance(from->coordinates, to->coordinates) * 2;
            }
        }

        route_info.unique_stops_count = UniqueStopsCount(route_number);
        route_info.route_length = route_length;
        route_info.curvature = route_length / route_length_geograf;

        return route_info;
    }
    
    size_t TransportCatalogue::UniqueStopsCount(const std::string& route_number) const
    {
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop : busname_to_bus_.at(route_number)->stops.value())
        {
            unique_stops.insert(stop);
        }
        return unique_stops.size();
    }

    const std::unordered_set<std::string> TransportCatalogue::GetBusesOnStop(const std::string& stop_name) const
    {
        return stopname_to_stop_.at(stop_name)->buses;
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, const int distance) 
    {
        stop_distances_[{from, to}] = distance;
    }

    int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const 
    {
        if (stop_distances_.count({ from, to })) return stop_distances_.at({ from, to });
        else if (stop_distances_.count({ to, from })) return stop_distances_.at({ to, from });
        else return 0;
    }

}
