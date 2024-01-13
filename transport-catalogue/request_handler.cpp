#include "request_handler.h"

std::optional<transport_catalogue::BusStat> RequestHandler::RouteInformation(const std::string_view bus_number) const {
    const transport_catalogue::Bus* bus = catalogue_.FindRoute(bus_number);

    if (!bus) {
        throw std::invalid_argument("bus not found");
    }

    return catalogue_.CalculateBusStat(bus);
}

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name)->buses_by_stop;
}

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.FindRoute(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetBusesOnStop());
}