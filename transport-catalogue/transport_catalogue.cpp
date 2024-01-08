#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view stop_name, const geo::Coordinates coordinates) {
        all_stops_.push_back({ std::string(stop_name), coordinates, {} });
        stopname_to_stop_[all_stops_.back().name] = &all_stops_.back();
    }

    void TransportCatalogue::AddRoute(std::string_view bus_number, const std::vector<const Stop*> stops, bool is_circle) {
        all_buses_.push_back({ std::string(bus_number), stops, is_circle });
        busname_to_bus_[all_buses_.back().number] = &all_buses_.back();
        for (const auto& route_stop : stops) {
            for (auto& stop_ : all_stops_) {
                if (stop_.name == route_stop->name) stop_.buses_by_stop.insert(std::string(bus_number));
            }
        }
    }

    const Bus* TransportCatalogue::FindRoute(std::string_view bus_number) const {
        return busname_to_bus_.count(bus_number) ? busname_to_bus_.at(bus_number) : nullptr;
    }

    const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        return stopname_to_stop_.count(stop_name) ? stopname_to_stop_.at(stop_name) : nullptr;
    }

    size_t TransportCatalogue::UniqueStopsCount(std::string_view bus_number) const {
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop : busname_to_bus_.at(bus_number)->stops) {
            unique_stops.insert(stop->name);
        }
        return unique_stops.size();
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, const int distance) {
        stop_distances_[{from, to}] = distance;
    }

    int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
        if (stop_distances_.count({ from, to })) return stop_distances_.at({ from, to });
        else if (stop_distances_.count({ to, from })) return stop_distances_.at({ to, from });
        else return 0;
    }

    const std::map<std::string_view, const Bus*> TransportCatalogue::GetBusesOnStop() const {
        std::map<std::string_view, const Bus*> result;
        for (const auto& bus : busname_to_bus_) {
            result.emplace(bus);
        }
        return result;
    }
} // namespace transport_catalogue