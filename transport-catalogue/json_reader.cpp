#include "json_reader.h"
#include "json_builder.h"

using namespace std::literals;

enum ColorTypeRGB {
    TipeRgb = 3,
    TipeRgb_A = 4
};

const json::Node& JsonReader::GetBaseRequests() const {

    const auto& root = input_.GetRoot().AsDict();

    auto it = root.find("base_requests");

    if (it != root.end())
    {
        return it->second;
    }

    return dummy_;
}

const json::Node& JsonReader::GetStatRequests() const {

    const auto& root = input_.GetRoot().AsDict();

    auto it = root.find("stat_requests");

    if (it != root.end())
    {
        return it->second;
    }

    return dummy_;
}

const json::Node& JsonReader::GetRenderSettings() const {

    const auto& root = input_.GetRoot().AsDict();

    auto it = root.find("render_settings");

    if (it != root.end())
    {
        return it->second;
    }

    return dummy_;
}

const json::Node& JsonReader::GetRoutingSettings() const {
    
    const auto& root = input_.GetRoot().AsDict();

    auto it = root.find("routing_settings");
    
    if (it != root.end())
    {
        return it->second;
    }

    return dummy_;
}

void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();

        if (type == "Stop") {
            result.push_back(PrintStop(request_map, rh).AsDict());
        }
        else if (type == "Bus") {
            result.push_back(PrintRoute(request_map, rh).AsDict());
        }
        else if (type == "Map") {
            result.push_back(PrintMap(request_map, rh).AsDict());
        }
        else if (type == "Route") {
            result.push_back(PrintRouting(request_map, rh).AsDict());
        }
    }

    json::Print(json::Document{ result }, std::cout);
}

void JsonReader::FillCatalogueStop(const json::Array& stops_array, transport_catalogue::TransportCatalogue& catalogue) {
    for (auto& request_stops : stops_array) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    FillStopDistances(catalogue);
}

void JsonReader::FillCatalogueBus(const json::Array& buses_array, transport_catalogue::TransportCatalogue& catalogue) {
    for (auto& request_bus : buses_array) {
        const auto& request_bus_map = request_bus.AsDict();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = FillRoute(request_bus_map, catalogue);
            catalogue.AddRoute(bus_number, stops, circular_route);
        }
    }
}

void JsonReader::FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();

    FillCatalogueStop(arr, catalogue);
    FillCatalogueBus(arr, catalogue);
}

std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::FillStop(const json::Dict& request_map) const {
    std::string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = request_map.at("road_distances").AsDict();
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    return { stop_name, coordinates, stop_distances };
}

void JsonReader::FillStopDistances(transport_catalogue::TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue.FindStop(stop_name);
                auto to = catalogue.FindStop(to_name);
                catalogue.SetDistance(from, to, dist);
            }
        }
    }
}

std::tuple<std::string_view, std::vector<const transport_catalogue::Stop*>, bool> JsonReader::FillRoute(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const transport_catalogue::Stop*> stops;
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString()));
    }
    bool circular_route = request_map.at("is_roundtrip").AsBool();

    return { bus_number, stops, circular_route };
}

svg::Color JsonReader::ParseColor(const json::Node& colorNode) const {
    if (colorNode.IsString()) {
        return colorNode.AsString();
    }
    else if (colorNode.IsArray()) {
        const json::Array& colorArray = colorNode.AsArray();
        if (colorArray.size() == ColorTypeRGB::TipeRgb) {
            return svg::Rgb(colorArray[0].AsInt(), colorArray[1].AsInt(), colorArray[2].AsInt());
        }
        else if (colorArray.size() == ColorTypeRGB::TipeRgb_A) {
            return svg::Rgba(colorArray[0].AsInt(), colorArray[1].AsInt(), colorArray[2].AsInt(), colorArray[3].AsDouble());
        }
        else {
            throw std::logic_error("Invalid color array size");
        }
    }
    else {
        throw std::logic_error("Invalid color type");
    }
}

renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

    render_settings.underlayer_color = ParseColor(request_map.at("underlayer_color"));

    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();

    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        render_settings.color_palette.push_back(ParseColor(color_element));
    }
    
    return render_settings;
}

transport_catalogue::Router JsonReader::FillRoutingSettings(const json::Node& settings) const {
    return {
        settings.AsDict().at("bus_wait_time").AsInt(),
        settings.AsDict().at("bus_velocity").AsDouble()
    };
}

const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& route_number = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsBusNumber(route_number)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        result["curvature"] = rh.RouteInformation(route_number)->curvature;
        result["route_length"] = rh.RouteInformation(route_number)->route_length;
        result["stop_count"] = static_cast<int>(rh.RouteInformation(route_number)->stops_count);
        result["unique_stop_count"] = static_cast<int>(rh.RouteInformation(route_number)->unique_stops_count);
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& stop_name = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        json::Array buses;
        for (auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result["buses"] = buses;
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{ result };
}

const json::Node JsonReader::PrintRouting(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const int id = request_map.at("id"s).AsInt();
    const std::string_view stop_from = request_map.at("from"s).AsString();
    const std::string_view stop_to = request_map.at("to"s).AsString();
    const auto& routing = rh.GetOptimalRoute(stop_from, stop_to);

    if (!routing) {
        result = json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    else {
        json::Array items;
        double total_time = 0.0;
        items.reserve(routing.value().edges.size());
        for (auto& edge_id : routing.value().edges) {
            const graph::Edge<double> edge = rh.GetRouterGraph().GetEdge(edge_id);
            if (edge.quality == 0) {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                    .Key("stop_name"s).Value(edge.name)
                    .Key("time"s).Value(edge.weight)
                    .Key("type"s).Value("Wait"s)
                    .EndDict()
                    .Build()));

                total_time += edge.weight;
            }
            else {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                    .Key("bus"s).Value(edge.name)
                    .Key("span_count"s).Value(static_cast<int>(edge.quality))
                    .Key("time"s).Value(edge.weight)
                    .Key("type"s).Value("Bus"s)
                    .EndDict()
                    .Build()));

                total_time += edge.weight;
            }
        }

        result = json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("total_time"s).Value(total_time)
            .Key("items"s).Value(items)
            .EndDict()
            .Build();
    }

    return result;
}