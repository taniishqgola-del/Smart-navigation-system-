// ============================================================
//  SMART CAMPUS NAVIGATION SYSTEM  -  C++ REST API Backend
//  Student Name : Tanishq Gola
//  Roll Number  : 25/SET/BCSE/519
//
//  The adjacency matrix, campus connections, distances and the
//  Dijkstra algorithm below are taken from the original console
//  project. Only the console input/output was replaced by a
//  REST API (Crow framework) so a website can use it.
// ============================================================

#include "crow.h"
#include "crow/middlewares/cors.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Total campus locations
const int N = 8;

// Distance for locations that are not connected
const int INF = 999999;

// Campus locations
string places[N] = {
    "Main Gate",
    "Main Institute",
    "Library",
    "Canteen",
    "Sports Science Center",
    "Dental College",
    "M Block",
    "Bus Parking"
};

// Graph using Adjacency Matrix
int graph[N][N] = {0};

// Function to create campus connections
void createCampus()
{
    // Main Gate connections
    graph[0][1] = 120;
    graph[1][0] = 120;

    graph[0][7] = 90;
    graph[7][0] = 90;

    // Main Institute connections
    graph[1][2] = 180;
    graph[2][1] = 180;

    graph[1][3] = 100;
    graph[3][1] = 100;

    graph[1][4] = 220;
    graph[4][1] = 220;

    graph[1][6] = 150;
    graph[6][1] = 150;

    // Other connections
    graph[2][3] = 140;
    graph[3][2] = 140;

    graph[2][5] = 250;
    graph[5][2] = 250;

    graph[2][6] = 130;
    graph[6][2] = 130;

    graph[4][5] = 300;
    graph[5][4] = 300;

    graph[3][7] = 170;
    graph[7][3] = 170;
}

// Result of a shortest path search
struct RouteResult
{
    bool reachable = false;
    int totalDistance = 0;
    vector<int> path; // location indices, source first
};

// Dijkstra's Shortest Path Algorithm (same logic as the console project)
RouteResult findShortestPath(int source, int destination)
{
    int distance[N];
    bool visited[N] = {false};
    int parent[N];

    // Initial values
    for (int i = 0; i < N; i++)
    {
        distance[i] = INF;
        parent[i] = -1;
    }

    distance[source] = 0;

    // Dijkstra Algorithm
    for (int count = 0; count < N; count++)
    {
        int u = -1;
        int minimum = INF;

        // Find nearest unvisited location
        for (int i = 0; i < N; i++)
        {
            if (!visited[i] && distance[i] < minimum)
            {
                minimum = distance[i];
                u = i;
            }
        }

        // Stop if no location is reachable
        if (u == -1)
        {
            break;
        }

        visited[u] = true;

        // Check connected locations
        for (int v = 0; v < N; v++)
        {
            if (graph[u][v] > 0 && !visited[v])
            {
                int newDistance = distance[u] + graph[u][v];

                if (newDistance < distance[v])
                {
                    distance[v] = newDistance;
                    parent[v] = u;
                }
            }
        }
    }

    RouteResult result;

    // Check whether destination is reachable
    if (distance[destination] == INF)
    {
        return result; // reachable = false
    }

    // Store shortest route (built backwards, then reversed)
    int current = destination;
    while (current != -1)
    {
        result.path.insert(result.path.begin(), current);
        current = parent[current];
    }

    result.reachable = true;
    result.totalDistance = distance[destination];
    return result;
}

// ---------------------------------------------------------------
//  Helpers for the REST API
// ---------------------------------------------------------------

static crow::response jsonResponse(int code, const crow::json::wvalue &body)
{
    crow::response res(code, body.dump());
    res.set_header("Content-Type", "application/json");
    return res;
}

static crow::response errorResponse(int code, const string &message)
{
    crow::json::wvalue body;
    body["success"] = false;
    body["error"] = message;
    return jsonResponse(code, body);
}

// Finds the frontend folder so the website can also be served by this server
static string locateFrontend(int argc, char *argv[])
{
    vector<string> candidates;
    if (argc > 1)
        candidates.push_back(argv[1]);
    candidates.push_back("../frontend");
    candidates.push_back("../../frontend");
    candidates.push_back("frontend");

    for (const auto &c : candidates)
    {
        if (filesystem::exists(filesystem::path(c) / "index.html"))
            return filesystem::absolute(c).lexically_normal().string();
    }
    return "";
}

int main(int argc, char *argv[])
{
    createCampus();

    const string frontendDir = locateFrontend(argc, argv);

    crow::App<crow::CORSHandler> app;

    // Allow the frontend to call the API even if it is opened from
    // another origin (e.g. VS Code Live Server or a file:// page)
    auto &cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST, crow::HTTPMethod::OPTIONS)
        .headers("Content-Type");

    // ---------------- GET /api/health ----------------
    CROW_ROUTE(app, "/api/health")
    ([]() {
        crow::json::wvalue body;
        body["status"] = "ok";
        body["service"] = "Smart Campus Navigation System";
        body["algorithm"] = "Dijkstra's Shortest Path";
        return jsonResponse(200, body);
    });

    // ---------------- GET /api/locations ----------------
    // Returns all locations and every connection of the campus graph
    CROW_ROUTE(app, "/api/locations")
    ([]() {
        crow::json::wvalue body;
        body["success"] = true;

        vector<crow::json::wvalue> locations;
        for (int i = 0; i < N; i++)
        {
            crow::json::wvalue loc;
            loc["id"] = i;
            loc["name"] = places[i];
            locations.push_back(std::move(loc));
        }
        body["locations"] = std::move(locations);

        // each undirected connection is listed once
        vector<crow::json::wvalue> connections;
        for (int i = 0; i < N; i++)
        {
            for (int j = i + 1; j < N; j++)
            {
                if (graph[i][j] > 0)
                {
                    crow::json::wvalue edge;
                    edge["from"] = i;
                    edge["to"] = j;
                    edge["distance"] = graph[i][j];
                    connections.push_back(std::move(edge));
                }
            }
        }
        body["connections"] = std::move(connections);

        return jsonResponse(200, body);
    });

    // ---------------- POST /api/route ----------------
    // Body: { "source": 0, "destination": 4 }   (location ids 0-7)
    CROW_ROUTE(app, "/api/route").methods(crow::HTTPMethod::POST)([](const crow::request &req) {
        auto data = crow::json::load(req.body);

        if (!data || !data.has("source") || !data.has("destination"))
            return errorResponse(400, "Request body must be JSON with 'source' and 'destination'.");

        if (data["source"].t() != crow::json::type::Number ||
            data["destination"].t() != crow::json::type::Number)
            return errorResponse(400, "'source' and 'destination' must be numbers.");

        int source = static_cast<int>(data["source"].i());
        int destination = static_cast<int>(data["destination"].i());

        if (source < 0 || source >= N || destination < 0 || destination >= N)
            return errorResponse(400, "Invalid location number. Use ids 0 to 7.");

        if (source == destination)
            return errorResponse(400, "You are already at your destination!");

        RouteResult result = findShortestPath(source, destination);

        if (!result.reachable)
            return errorResponse(404, "No route available!");

        crow::json::wvalue body;
        body["success"] = true;
        body["source"] = places[source];
        body["destination"] = places[destination];
        body["totalDistance"] = result.totalDistance;

        vector<crow::json::wvalue> pathIds, pathNames, segments;
        for (size_t i = 0; i < result.path.size(); i++)
        {
            int id = result.path[i];
            pathIds.push_back(crow::json::wvalue(id));
            pathNames.push_back(crow::json::wvalue(places[id]));

            if (i + 1 < result.path.size())
            {
                int next = result.path[i + 1];
                crow::json::wvalue seg;
                seg["from"] = id;
                seg["to"] = next;
                seg["fromName"] = places[id];
                seg["toName"] = places[next];
                seg["distance"] = graph[id][next];
                segments.push_back(std::move(seg));
            }
        }
        body["path"] = std::move(pathIds);
        body["pathNames"] = std::move(pathNames);
        body["segments"] = std::move(segments);

        return jsonResponse(200, body);
    });

    // ---------------- Serve the website (optional convenience) ----------------
    if (!frontendDir.empty())
    {
        cout << "Serving website from: " << frontendDir << endl;

        CROW_ROUTE(app, "/")
        ([frontendDir](const crow::request &, crow::response &res) {
            res.set_static_file_info_unsafe(frontendDir + "/index.html");
            res.end();
        });

        CROW_ROUTE(app, "/<path>")
        ([frontendDir](const crow::request &, crow::response &res, const string &path) {
            // block directory traversal
            if (path.find("..") != string::npos)
            {
                res.code = 400;
                res.end();
                return;
            }
            res.set_static_file_info_unsafe(frontendDir + "/" + path);
            res.end();
        });
    }
    else
    {
        cout << "Frontend folder not found - API only mode." << endl;
    }

    cout << "\n=====================================================\n";
    cout << "  Smart Campus Navigation System backend is running\n";
    cout << "  Open the website:  http://localhost:18080\n";
    cout << "  API health check:  http://localhost:18080/api/health\n";
    cout << "=====================================================\n";

    app.bindaddr("127.0.0.1").port(18080).multithreaded().run();
    return 0;
}
