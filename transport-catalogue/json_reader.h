#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input))
    {
    }

    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;

    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;

    void FillCatalogueStop(const json::Array& stops_array, transport_catalogue::TransportCatalogue& catalogue);
    void FillCatalogueBus(const json::Array& buses_array, transport_catalogue::TransportCatalogue& catalogue);

    void FillCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

    svg::Color ParseColor(const json::Node& colorNode) const;

    const json::Node PrintRoute(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintStop(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintMap(const json::Dict& request_map, RequestHandler& rh) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> FillStop(const json::Dict& request_map) const;
    void FillStopDistances(transport_catalogue::TransportCatalogue& catalogue) const;
    std::tuple<std::string_view, std::vector<const transport_catalogue::Stop*>, bool> FillRoute(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const;
};