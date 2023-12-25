#include "stat_reader.h"

#include <iomanip>
#include <iostream>

namespace transport_catalogue
{
    void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output)
    {
        std::string route(std::move(request));
        route.erase(0, 4);

        std::string stop(std::move(request));
        stop.erase(4, route.size());

        std::string stop_name(std::move(request));
        stop_name.erase(0, 5);

        if (tansport_catalogue.FindRoute(route))
        {
            output << "Bus " << route << ": " << tansport_catalogue.RouteInformation(route).stops_count << " stops on route, "
                << tansport_catalogue.RouteInformation(route).unique_stops_count << " unique stops, " << std::setprecision(6)
                << tansport_catalogue.RouteInformation(route).route_length << " route length\n";
        }
        else if (stop == "Stop")
        {
            if (tansport_catalogue.FindStop(stop_name))
            {
                output << "Stop " << stop_name << ": ";
                std::set<std::string> buses = tansport_catalogue.GetBusesOnStop(stop_name);
                if (!buses.empty())
                {
                    output << "buses ";
                    for (const auto& bus : buses)
                    {
                        output << bus << " ";
                    }
                    output << "\n";
                }
                else
                {
                    output << "no buses\n";
                }
            }
            else
            {
                output << "Stop " << stop_name << ": not found\n";
            }
        }
        else
        {
            output << "Bus " << route << ": not found\n";
        }
    }
}