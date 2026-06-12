#include <bits/stdc++.h>
using namespace std;

// Utility types and helper functions
typedef int StopID;
struct Point
{
    double x, y;
};
// Calculate distance between two points
double euclidean(const Point &a, const Point &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}
// Remove whitespace from string
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Simple logging
struct Logger
{
    deque<string> q;
    size_t maxlen = 1000;
    void log(const string &s)
    {
        q.push_back(s);
        if (q.size() > maxlen)
            q.pop_front();
    }
    void print_recent(size_t n = 20)
    {
        cout << "Recent logs (last " << n << ":" << endl;
        size_t start = 0;
        if (q.size() > n)
            start = q.size() - n;
        for (size_t i = start; i < q.size(); i++)
            cout << q[i] << endl;
    }
} logger;

// Trie for autocomplete
struct TrieNode
{
    bool end;
    unordered_map<char, TrieNode *> nxt;
    TrieNode() { end = false; }
};
struct Trie
{
    TrieNode *root;
    Trie() { root = new TrieNode(); }
    void insert(const string &s)
    {
        TrieNode *cur = root;
        for (int i = 0; i < s.length(); i++)
        {
            char ch = s[i];
            if (cur->nxt.find(ch) == cur->nxt.end())
                cur->nxt[ch] = new TrieNode();
            cur = cur->nxt[ch];
        }
        cur->end = true;
    }
    void collect(TrieNode *node, const string &prefix, vector<string> &out)
    {
        if (node == NULL)
            return;
        if (node->end)
            out.push_back(prefix);
        for (auto p : node->nxt)
            collect(p.second, prefix + p.first, out);
    }
    vector<string> suggest(const string &prefix)
    {
        TrieNode *cur = root;
        for (int i = 0; i < prefix.length(); i++)
        {
            char c = prefix[i];
            if (cur->nxt.find(c) == cur->nxt.end())
                return vector<string>();
            cur = cur->nxt[c];
        }
        vector<string> out;
        collect(cur, prefix, out);
        return out;
    }
};

// Stops and edges
struct Stop
{
    StopID id;
    string name;
    Point loc;
    Stop()
    {
        id = -1;
        name = "";
        loc.x = 0;
        loc.y = 0;
    }
    Stop(StopID id_, const string &name_, Point loc_)
    {
        id = id_;
        name = name_;
        loc = loc_;
    }
};
struct Edge
{
    StopID to;
    double weight;
    Edge()
    {
        to = 0;
        weight = 0;
    }
    Edge(StopID t, double w)
    {
        to = t;
        weight = w;
    }
};

struct Graph
{
    vector<Stop> stops;
    unordered_map<string, StopID> nameToId;
    vector<vector<Edge>> adj;

    Graph() {}

    StopID add_stop(const string &name, double x = 0.0, double y = 0.0)
    {
        string tn = trim(name);
        if (nameToId.count(tn))
            return nameToId[tn];
        StopID id = (StopID)stops.size();
        stops.push_back(Stop(id, tn, Point{x, y}));
        nameToId[tn] = id;
        adj.emplace_back();
        logger.log(string("Added stop: ") + tn + " (id=" + to_string(id) + ")");
        return id;
    }

    bool has_stop(const string &name) const
    {
        return nameToId.count(trim(name)) > 0;
    }

    StopID get_id(const string &name) const
    {
        auto t = nameToId.find(trim(name));
        if (t == nameToId.end())
            return -1;
        return t->second;
    }

    string get_name(StopID id) const
    {
        if (id < 0 || id >= (StopID)stops.size())
            return "";
        return stops[id].name;
    }

    Point get_loc(StopID id) const
    {
        if (id < 0 || id >= (StopID)stops.size())
            return Point{0, 0};
        return stops[id].loc;
    }

    void set_location(StopID id, double x, double y)
    {
        if (id < 0 || id >= (StopID)stops.size())
            return;
        stops[id].loc = Point{x, y};
    }

    void add_edge(const string &a, const string &b, double weight, bool bidir = true)
    {
        StopID u = add_stop(a);
        StopID v = add_stop(b);
        add_edge(u, v, weight, bidir);
    }

    void add_edge(StopID u, StopID v, double weight, bool bidir = true)
    {
        if (u < 0 || v < 0 || u >= (StopID)stops.size() || v >= (StopID)stops.size())
            return;
        adj[u].push_back(Edge(v, weight));
        if (bidir)
            adj[v].push_back(Edge(u, weight));
        logger.log(string("Added edge: ") + stops[u].name + " <-> " + stops[v].name + " (" + to_string(weight) + ")");
    }

    vector<pair<StopID, double>> neighbors(StopID u) const
    {
        vector<pair<StopID, double>> out;
        if (u < 0 || u >= (StopID)adj.size())
            return out;
        for (size_t i = 0; i < adj[u].size(); ++i)
            out.push_back(make_pair(adj[u][i].to, adj[u][i].weight));
        return out;
    }

    size_t size() const { return stops.size(); }

    // Persist graph to files (stops and edges)
    bool save_to(const string &stopsFile, const string &edgesFile) const
    {
        ofstream sf(stopsFile.c_str());
        ofstream ef(edgesFile.c_str());
        if (!sf || !ef)
            return false;
        for (size_t i = 0; i < stops.size(); ++i)
        {
            const Stop &s = stops[i];
            sf << s.id << "\t" << s.name << "\t" << s.loc.x << "\t" << s.loc.y << "\n";
        }
        for (size_t u = 0; u < adj.size(); ++u)
        {
            for (size_t j = 0; j < adj[u].size(); ++j)
            {
                const Edge &e = adj[u][j];
                ef << u << "\t" << e.to << "\t" << e.weight << "\n";
            }
        }
        sf.close();
        ef.close();
        logger.log(string("Saved graph to files: ") + stopsFile + " , " + edgesFile);
        return true;
    }

    bool load_from(const string &stopsFile, const string &edgesFile)
    {
        ifstream sf(stopsFile.c_str());
        ifstream ef(edgesFile.c_str());
        if (!sf || !ef)
            return false;
        stops.clear();
        nameToId.clear();
        adj.clear();

        string line;
        while (getline(sf, line))
        {
            if (trim(line).empty())
                continue;
            stringstream ss(line);
            int id;
            string name;
            double x, y;
            ss >> id;
            ss >> ws;

            // read remainder of line and split by tabs or spaces
            string rest;
            getline(ss, rest);

            if (rest.find('\t') != string::npos)
            {
                // Parse tab-separated format
                stringstream rr(rest);
                getline(rr, name, '\t');
                rr >> x >> y;
            }
            else
            {
                // Parse space-separated format as fallback
                stringstream rr(rest);
                rr >> name >> x >> y;
            }
            if ((int)stops.size() <= id)
                stops.resize(id + 1);
            stops[id] = Stop(id, name, Point{x, y});
            nameToId[name] = id;
            if (adj.size() <= (size_t)id)
                adj.resize(id + 1);
        }

        while (getline(ef, line))
        {
            if (trim(line).empty())
                continue;
            stringstream ss(line);
            int u, v;
            double w;
            ss >> u >> v >> w;
            if (u >= 0 && v >= 0)
            {
                if ((size_t)u >= adj.size() || (size_t)v >= adj.size())
                    continue;
                adj[u].push_back(Edge(v, w));
            }
        }
        logger.log(string("Loaded graph from files: ") + stopsFile + " , " + edgesFile);
        return true;
    }
};

// Dijkstra
pair<vector<double>, vector<int>> dijkstra(const Graph &g, StopID src)
{
    size_t n = g.size();
    const double INF = 1e18;
    vector<double> dist(n, INF);
    vector<int> prev(n, -1);
    typedef pair<double, StopID> PDI;
    priority_queue<PDI, vector<PDI>, greater<PDI>> pq;
    if (src < 0 || src >= (StopID)n)
        return make_pair(dist, prev);
    dist[src] = 0;
    pq.push(make_pair(0.0, src));
    while (!pq.empty())
    {
        PDI top = pq.top();
        pq.pop();
        double d = top.first;
        StopID u = top.second;
        if (d > dist[u])
            continue;
        vector<pair<StopID, double>> nbrs = g.neighbors(u);
        for (size_t i = 0; i < nbrs.size(); ++i)
        {
            StopID v = nbrs[i].first;
            double w = nbrs[i].second;
            if (dist[v] > dist[u] + w)
            {
                dist[v] = dist[u] + w;
                prev[v] = (int)u;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
    return make_pair(dist, prev);
}

// Build path from source to target using parent/predecessor array
vector<StopID> reconstruct_path(const vector<int> &prev, StopID target)
{
    vector<StopID> path;
    for (int at = (int)target; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    return path;
}

// A* pathfinding result - includes success status, path, and estimated cost
struct AStarResult
{
    bool found;          // Whether a path was found
    vector<StopID> path; // Sequence of stops in the path
    double cost;         // Total travel time estimate
};
// A* pathfinding algorithm using geographic heuristic (Euclidean distance)
AStarResult astar(const Graph &g, StopID src, StopID dest, double avgSpeedKmPerHr = 40.0, double kmToMinFactor = 1.0)
{
    size_t n = g.size();
    if (src < 0 || dest < 0 || src >= (StopID)n || dest >= (StopID)n)
        return AStarResult{false, vector<StopID>(), 0.0};
    vector<double> gscore(n, 1e18), fscore(n, 1e18);
    vector<int> cameFrom(n, -1);
    // Heuristic: estimate remaining time based on straight-line distance
    auto heuristic = [&](StopID a, StopID b) -> double
    {
        Point pa = g.get_loc(a), pb = g.get_loc(b);
        double km = euclidean(pa, pb);
        double hours = (avgSpeedKmPerHr <= 0) ? 0 : (km / avgSpeedKmPerHr);
        return hours * 60.0; // Convert to minutes
    };
    typedef pair<double, StopID> P;
    priority_queue<P, vector<P>, greater<P>> openSet;
    gscore[src] = 0;
    fscore[src] = heuristic(src, dest);
    openSet.push(make_pair(fscore[src], src));
    while (!openSet.empty())
    {
        P top = openSet.top();
        openSet.pop();
        double curf = top.first;
        StopID cur = top.second;
        if (cur == dest)
        {
            vector<StopID> path = reconstruct_path(cameFrom, dest);
            return AStarResult{true, path, gscore[dest]};
        }
        vector<pair<StopID, double>> nbrs = g.neighbors(cur);
        for (size_t i = 0; i < nbrs.size(); ++i)
        {
            StopID v = nbrs[i].first;
            double w = nbrs[i].second;
            double tentative = gscore[cur] + w;
            if (tentative < gscore[v])
            {
                cameFrom[v] = (int)cur;
                gscore[v] = tentative;
                fscore[v] = tentative + heuristic(v, dest);
                openSet.push(make_pair(fscore[v], v));
            }
        }
    }
    return AStarResult{false, vector<StopID>(), 0.0};
}
// Prim's MST
pair<double, vector<pair<StopID, StopID>>> prim_mst(const Graph &g)
{
    size_t n = g.size();
    if (n == 0)
        return make_pair(0.0, vector<pair<StopID, StopID>>());
    vector<bool> inMST(n, false);
    vector<double> key(n, 1e18);
    vector<int> parent(n, -1);
    typedef pair<double, StopID> P;
    priority_queue<P, vector<P>, greater<P>> pq;
    key[0] = 0;
    pq.push(make_pair(0.0, 0));
    while (!pq.empty())
    {
        P top = pq.top();
        pq.pop();
        double k = top.first;
        StopID u = top.second;
        if (inMST[u])
            continue;
        inMST[u] = true;
        vector<pair<StopID, double>> nbrs = g.neighbors(u);
        for (size_t i = 0; i < nbrs.size(); ++i)
        {
            StopID v = nbrs[i].first;
            double w = nbrs[i].second;
            if (!inMST[v] && key[v] > w)
            {
                key[v] = w;
                parent[v] = (int)u;
                pq.push(make_pair(key[v], v));
            }
        }
    }
    double total = 0;
    vector<pair<StopID, StopID>> edges;
    for (size_t v = 1; v < n; ++v)
    {
        if (parent[v] != -1)
        {
            edges.push_back(make_pair((StopID)parent[v], (StopID)v));
            // find weight
            vector<pair<StopID, double>> nbrs = g.neighbors(parent[v]);
            for (size_t i = 0; i < nbrs.size(); ++i)
            {
                if (nbrs[i].first == (StopID)v)
                {
                    total += nbrs[i].second;
                    break;
                }
            }
        }
    }
    return make_pair(total, edges);
}

// Bus entity with route and position tracking
struct Bus
{
    string busId;
    vector<StopID> route; // Ordered sequence of stops
    int currentIndex;     // Current position in route
    double speed;         // Speed in km/h (informational)
    bool active;          // Whether bus is currently running
    Bus() : busId(""), currentIndex(0), speed(40.0), active(false) {}
    Bus(const string &id, const vector<StopID> &r, double sp = 40.0) : busId(id), route(r), currentIndex(0), speed(sp), active(true) {}
    StopID current_stop() const
    {
        if (route.empty())
            return -1;
        int idx = currentIndex;
        if (idx < 0)
            idx = 0;
        if (idx >= (int)route.size())
            idx = (int)route.size() - 1;
        return route[idx];
    }
    StopID next_stop() const
    {
        if (route.empty())
            return -1;
        if (currentIndex + 1 >= (int)route.size())
            return route.back();
        return route[currentIndex + 1];
    }
    bool move_next()
    {
        if (!active)
            return false;
        if (route.empty())
            return false;
        if (currentIndex + 1 < (int)route.size())
        {
            currentIndex++;
            return true;
        }
        else
        {
            active = false;
            return false;
        }
    }
    string info(const Graph &g) const
    {
        stringstream ss;
        ss << "[" << busId << "] ";
        ss << "At: ";
        StopID cs = current_stop();
        if (cs >= 0)
            ss << g.get_name(cs);
        else
            ss << "N/A";
        ss << " (idx " << currentIndex << "/" << (route.empty() ? 0 : route.size() - 1) << ")";
        ss << (active ? " RUNNING" : " STOPPED");
        return ss.str();
    }
    string serialize() const
    {
        stringstream ss;
        ss << busId << "|" << speed << "|" << currentIndex;
        for (size_t i = 0; i < route.size(); ++i)
            ss << "|" << route[i];
        return ss.str();
    }
    static Bus deserialize(const string &line)
    {
        vector<string> parts;
        string cur;
        stringstream ss(line);
        while (getline(ss, cur, '|'))
            parts.push_back(cur);
        Bus bus;
        if (parts.size() >= 3)
        {
            bus.busId = parts[0];
            bus.speed = atof(parts[1].c_str());
            bus.currentIndex = atoi(parts[2].c_str());
            for (size_t i = 3; i < parts.size(); ++i)
                bus.route.push_back((StopID)atoi(parts[i].c_str()));
            bus.active = true;
        }
        return bus;
    }
};


// Bus system
struct BusSystem
{
    Graph g;
    unordered_map<string, Bus> buses;
    unordered_map<StopID, vector<string>> stopToBuses;
    queue<string> history; // Event log of bus movements
    size_t maxHistory;
    Trie trie; // Prefix search for stop names

    BusSystem() : maxHistory(1000) {}

    void rebuild_stop_index()
    {
        stopToBuses.clear();
        for (auto it = buses.begin(); it != buses.end(); ++it)
        {
            const Bus &b = it->second;
            if (b.route.empty())
                continue;
            StopID sid = b.current_stop();
            if (sid >= 0)
                stopToBuses[sid].push_back(it->first);
        }
    }

    bool add_stop_with_location(const string &name, double x, double y)
    {
        StopID id = g.add_stop(name, x, y);
        trie.insert(name);
        return id >= 0;
    }

    bool add_route_by_names(const string &a, const string &b, double minutes)
    {
        g.add_edge(a, b, minutes, true);
        trie.insert(trim(a));
        trie.insert(trim(b));
        return true;
    }

    bool add_bus(const string &busId, const vector<string> &routeNames, double speed = 40.0)
    {
        vector<StopID> r;
        for (size_t i = 0; i < routeNames.size(); ++i)
        {
            const string &nm = routeNames[i];
            StopID id = g.get_id(nm);
            if (id == (StopID)-1)
            {
                id = g.add_stop(nm);
                trie.insert(nm);
            }
            r.push_back(id);
        }
        Bus bus(busId, r, speed);
        buses[busId] = bus;
        rebuild_stop_index();
        logger.log(string("Added bus: ") + busId + " with " + to_string(r.size()) + " stops");
        return true;
    }

    bool move_bus_one_step(const string &busId)
    {
        if (!buses.count(busId))
            return false;
        Bus &bus = buses[busId];
        StopID prev = bus.current_stop();
        bool moved = bus.move_next();
        StopID curr = bus.current_stop();
        string entry = string("[") + current_time_str() + "] " + busId + " : " + g.get_name(prev) + " -> " + g.get_name(curr);
        push_history(entry);
        logger.log(entry);
        rebuild_stop_index();
        return moved;
    }

    void move_all_buses_one_step()
    {
        for (auto it = buses.begin(); it != buses.end(); ++it)
        {
            Bus &bus = it->second;
            StopID prev = bus.current_stop();
            if (bus.active)
            {
                bool moved = bus.move_next();
                StopID curr = bus.current_stop();
                string entry = string("[") + current_time_str() + "] " + bus.busId + " : " + g.get_name(prev) + " -> " + g.get_name(curr);
                push_history(entry);
                logger.log(entry);
            }
        }
        rebuild_stop_index();
    }

    vector<string> buses_at_stop(const string &stopName)
    {
        StopID id = g.get_id(stopName);
        vector<string> out;
        if (id == (StopID)-1)
            return out;
        if (stopToBuses.count(id))
            out = stopToBuses[id];
        return out;
    }

    // ETA between current bus location and some target stop: use Dijkstra from current stop
    double estimate_eta_for_bus(const string &busId, const string &targetStopName)
    {
        if (!buses.count(busId))
            return -1.0;
        StopID target = g.get_id(targetStopName);
        if (target == (StopID)-1)
            return -1.0;
        StopID src = buses[busId].current_stop();
        if (src == (StopID)-1)
            return -1.0;
        pair<vector<double>, vector<int>> res = dijkstra(g, src);
        vector<double> dist = res.first;
        if (dist[target] >= 1e17)
            return -1.0;
        return dist[target];
    }

    // estimate ETA between any two stops (names)
    double estimate_eta_between(const string &a, const string &b)
    {
        StopID sa = g.get_id(a);
        StopID sb = g.get_id(b);
        if (sa == (StopID)-1 || sb == (StopID)-1)
            return -1.0;
        pair<vector<double>, vector<int>> res = dijkstra(g, sa);
        vector<double> dist = res.first;
        if (dist[sb] >= 1e17)
            return -1.0;
        return dist[sb];
    }

    // find shortest path (Dijkstra) with names returned
    pair<double, vector<string>> shortest_path_names(const string &a, const string &b)
    {
        vector<string> emptyRes;
        StopID sa = g.get_id(a);
        StopID sb = g.get_id(b);
        if (sa == (StopID)-1 || sb == (StopID)-1)
            return make_pair(-1.0, emptyRes);
        pair<vector<double>, vector<int>> res = dijkstra(g, sa);
        vector<double> dist = res.first;
        vector<int> prev = res.second;
        if (dist[sb] > 1e17)
            return make_pair(-1.0, emptyRes);
        vector<StopID> path = reconstruct_path(prev, sb);
        vector<string> names;
        for (size_t i = 0; i < path.size(); ++i)
            names.push_back(g.get_name(path[i]));
        return make_pair(dist[sb], names);
    }

    // A* path (uses locations)
    pair<double, vector<string>> astar_names(const string &a, const string &b)
    {
        vector<string> emptyRes;
        StopID sa = g.get_id(a), sb = g.get_id(b);
        if (sa == (StopID)-1 || sb == (StopID)-1)
            return make_pair(-1.0, emptyRes);
        AStarResult res = astar(g, sa, sb);
        if (!res.found)
            return make_pair(-1.0, emptyRes);
        vector<string> names;
        for (size_t i = 0; i < res.path.size(); ++i)
            names.push_back(g.get_name(res.path[i]));
        return make_pair(res.cost, names);
    }

    // MST
    pair<double, vector<pair<string, string>>> mst_names()
    {
        pair<double, vector<pair<StopID, StopID>>> p = prim_mst(g);
        double total = p.first;
        vector<pair<StopID, StopID>> edges = p.second;
        vector<pair<string, string>> out;
        for (size_t i = 0; i < edges.size(); ++i)
        {
            out.push_back(make_pair(g.get_name(edges[i].first), g.get_name(edges[i].second)));
        }
        return make_pair(total, out);
    }

    // persistence for buses
    bool save_buses(const string &file)
    {
        ofstream ofs(file.c_str());
        if (!ofs)
            return false;
        for (auto it = buses.begin(); it != buses.end(); ++it)
        {
            ofs << it->second.serialize() << "\n";
        }
        ofs.close();
        logger.log(string("Saved buses to ") + file);
        return true;
    }

    bool load_buses(const string &file)
    {
        ifstream ifs(file.c_str());
        if (!ifs)
            return false;
        buses.clear();
        string line;
        while (getline(ifs, line))
        {
            if (trim(line).empty())
                continue;
            Bus b = Bus::deserialize(line);
            if (!b.busId.empty())
                buses[b.busId] = b;
        }
        ifs.close();
        rebuild_stop_index();
        logger.log(string("Loaded buses from ") + file);
        return true;
    }

    // basic history push (bounded)
    void push_history(const string &s)
    {
        history.push(s);
        if (history.size() > maxHistory)
            history.pop();
    }

    void print_history(size_t n = 20)
    {
        vector<string> items;
        queue<string> tmp = history;
        while (!tmp.empty())
        {
            items.push_back(tmp.front());
            tmp.pop();
        }
        cout << "---- Movement History (recent first) ----\n";
        for (int i = (int)items.size() - 1, c = 0; i >= 0 && c < (int)n; --i, ++c)
        {
            cout << items[i] << "\n";
        }
        cout << "-----------------------------------------\n";
    }

    string current_time_str() const
    {
        time_t t = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
        return string(buf);
    }

    // Suggest stops by prefix
    vector<string> suggest_stops(const string &prefix)
    {
        return trie.suggest(prefix);
    }

    // display system summary
    void print_summary()
    {
        cout << "===== Bus System Summary =====\n";
        cout << "Stops : " << g.size() << "\n";
        cout << "Buses : " << buses.size() << "\n";
        cout << "Recent Logs (top 5):\n";
        logger.print_recent(5);
        cout << "==============================\n";
    }
};//Demo dataset builder
void build_sample_data(BusSystem &sys)
{
    sys.add_stop_with_location("A", 0, 0);
    sys.add_stop_with_location("B", 2, 1);
    sys.add_stop_with_location("C", 4, 0.5);
    sys.add_stop_with_location("D", 3, -1.5);
    sys.add_stop_with_location("E", 1, -2);
    sys.add_stop_with_location("F", 5, -1);
    sys.add_stop_with_location("G", 6, 1.5);

    sys.add_route_by_names("A", "B", 6); // 6 minutes
    sys.add_route_by_names("B", "C", 5);
    sys.add_route_by_names("C", "F", 8);
    sys.add_route_by_names("A", "E", 7);
    sys.add_route_by_names("E", "D", 4);
    sys.add_route_by_names("D", "C", 6);
    sys.add_route_by_names("F", "G", 9);
    sys.add_route_by_names("C", "G", 7);

    sys.add_bus("BUS101", vector<string>{"A", "B", "C", "F", "G"}, 40.0);
    sys.add_bus("BUS202", vector<string>{"E", "D", "C", "B", "A"}, 35.0);
    sys.add_bus("BUS303", vector<string>{"G", "C", "D", "E", "A"}, 45.0);
}

//CLI
void show_menu()
{
    cout << "\n=== Bus Tracking System CLI ===\n";
    cout << "1. Show summary\n";
    cout << "2. List stops\n";
    cout << "3. Add stop\n";
    cout << "4. Add route (edge)\n";
    cout << "5. List buses\n";
    cout << "6. Add bus\n";
    cout << "7. Move all buses one step (simulate)\n";
    cout << "8. Move single bus one step\n";
    cout << "9. ETA between stops (Dijkstra)\n";
    cout << "10. ETA for bus -> stop\n";
    cout << "11. Shortest path (Dijkstra) show route\n";
    cout << "12. A* path (uses coordinates)\n";
    cout << "13. MST (Prim) suggestion\n";
    cout << "14. Suggest stops by prefix (Trie)\n";
    cout << "15. Save graph & buses to files\n";
    cout << "16. Load graph & buses from files\n";
    cout << "17. Show movement history\n";
    cout << "18. Show recent logger messages\n";
    cout << "19. Exit\n";
    cout << "Enter choice: " << endl;
}

void cli_loop(BusSystem &sys)
{
    while (true)
    {
        show_menu();
        int ch;
        if (!(cin >> ch))
        {
            cin.clear();
            string dummy;
            getline(cin, dummy);
            cout << "Invalid input. Try again.\n";
            continue;
        }
        if (ch == 1)
        {
            sys.print_summary();
        }
        else if (ch == 2)
        {
            cout << "Stops: \n";
            for (StopID i = 0; i < (StopID)sys.g.size(); ++i)
            {
                string s = sys.g.get_name(i);
                Point loc = sys.g.get_loc(i);
                cout << i << " : " << s << " (" << loc.x << "," << loc.y << ")\n";
            }
        }
        else if (ch == 3)
        {
            string name;
            double x, y;
            cout << "Enter stop name: ";
            cin >> ws;
            getline(cin, name);
            cout << "Enter x y coords (double): ";
            cin >> x >> y;
            sys.add_stop_with_location(name, x, y);
            cout << "Added.\n";
        }
        else if (ch == 4)
        {
            string a, b;
            double minutes;
            cout << "Enter stopA name: ";
            cin >> ws;
            getline(cin, a);
            cout << "Enter stopB name: ";
            cin >> ws;
            getline(cin, b);
            cout << "Enter travel time in minutes (double): ";
            cin >> minutes;
            sys.add_route_by_names(a, b, minutes);
            cout << "Edge added.\n";
        }
        else if (ch == 5)
        {
            cout << "Buses:\n";
            for (auto it = sys.buses.begin(); it != sys.buses.end(); ++it)
            {
                cout << it->second.info(sys.g) << "\n";
            }
        }
        else if (ch == 6)
        {
            string idline;
            cout << "Enter bus id: ";
            cin >> ws;
            getline(cin, idline);
            string routeStr;
            cout << "Enter comma-separated stops (names): ";
            cin >> ws;
            getline(cin, routeStr);
            double sp;
            cout << "Enter speed (km/h) [not used if edges are minutes, but recorded]: ";
            cin >> sp;
            vector<string> parts;
            string cur;
            stringstream ss(routeStr);
            while (getline(ss, cur, ','))
                parts.push_back(trim(cur));
            sys.add_bus(idline, parts, sp);
            cout << "Bus added.\n";
        }
        else if (ch == 7)
        {
            cout << "Moving all buses one step...\n";
            sys.move_all_buses_one_step();
            cout << "Done.\n";
        }
        else if (ch == 8)
        {
            string bid;
            cout << "Enter bus id: ";
            cin >> ws;
            getline(cin, bid);
            bool ok = sys.move_bus_one_step(bid);
            cout << (ok ? "Bus moved." : "Bus did not move (maybe at end or id wrong).") << "\n";
        }
        else if (ch == 9)
        {
            string a, b;
            cout << "Enter source stop: ";
            cin >> ws;
            getline(cin, a);
            cout << "Enter destination stop: ";
            cin >> ws;
            getline(cin, b);
            double eta = sys.estimate_eta_between(a, b);
            if (eta < 0)
                cout << "No path or stops not present.\n";
            else
                cout << "ETA (minutes): " << eta << "\n";
        }
        else if (ch == 10)
        {
            string bid, target;
            cout << "Enter bus id: ";
            cin >> ws;
            getline(cin, bid);
            cout << "Enter target stop name: ";
            cin >> ws;
            getline(cin, target);
            double eta = sys.estimate_eta_for_bus(bid, target);
            if (eta < 0)
                cout << "Could not compute ETA.\n";
            else
                cout << "ETA from current position (minutes): " << eta << "\n";
        }
        else if (ch == 11)
        {
            string a, b;
            cout << "Enter source stop: ";
            cin >> ws;
            getline(cin, a);
            cout << "Enter destination stop: ";
            cin >> ws;
            getline(cin, b);
            pair<double, vector<string>> res = sys.shortest_path_names(a, b);
            double cost = res.first;
            vector<string> path = res.second;
            if (cost < 0)
                cout << "No path.\n";
            else
            {
                cout << "Cost (minutes): " << cost << "\nPath: ";
                for (size_t i = 0; i < path.size(); ++i)
                {
                    cout << path[i] << (i + 1 < path.size() ? " -> " : "\n");
                }
            }
        }
        else if (ch == 12)
        {
            string a, b;
            cout << "Enter source stop: ";
            cin >> ws;
            getline(cin, a);
            cout << "Enter destination stop: ";
            cin >> ws;
            getline(cin, b);
            pair<double, vector<string>> res = sys.astar_names(a, b);
            double cost = res.first;
            vector<string> path = res.second;
            if (cost < 0)
                cout << "A* failed to find path.\n";
            else
            {
                cout << "A* cost (minutes approximated): " << cost << "\nPath: ";
                for (size_t i = 0; i < path.size(); ++i)
                {
                    cout << path[i] << (i + 1 < path.size() ? " -> " : "\n");
                }
            }
        }
        else if (ch == 13)
        {
            pair<double, vector<pair<string, string>>> res = sys.mst_names();
            double total = res.first;
            vector<pair<string, string>> edges = res.second;
            cout << "MST total weight (minutes): " << total << "\nEdges:\n";
            for (size_t i = 0; i < edges.size(); ++i)
                cout << edges[i].first << " - " << edges[i].second << "\n";
            cout << "Use this to plan minimal-cost physical cable/road layout (demo purpose).\n";
        }
        else if (ch == 14)
        {
            string pref;
            cout << "Enter prefix: ";
            cin >> ws;
            getline(cin, pref);
            vector<string> sug = sys.suggest_stops(pref);
            cout << "Suggestions:\n";
            for (size_t i = 0; i < sug.size(); ++i)
                cout << "  " << sug[i] << "\n";
        }
        else if (ch == 15)
        {
            string sf = "data/stops.txt", ef = "data/edges.txt", bf = "data/buses.txt";
            bool ok1 = sys.g.save_to(sf, ef);
            bool ok2 = sys.save_buses(bf);
            cout << "Saved graph: " << ok1 << " , buses: " << ok2 << "\n";
        }
        else if (ch == 16)
        {
            string sf = "data/stops.txt", ef = "data/edges.txt", bf = "data/buses.txt";
            bool ok1 = sys.g.load_from(sf, ef);
            bool ok2 = sys.load_buses(bf);
            cout << "Loaded graph: " << ok1 << " , buses: " << ok2 << "\n";
        }
        else if (ch == 17)
        {
            sys.print_history(30);
        }
        else if (ch == 18)
        {
            logger.print_recent(20);
        }
        else if (ch == 19)
        {
            cout << "Exiting. Goodbye!\n";
            break;
        }
        else
        {
            cout << "Unknown choice.\n";
        }
    }
}

// Main
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    BusSystem system;
    build_sample_data(system);

    cout << "Bus Tracking System (C++) - Demo backend\n";
    cout << "Sample data loaded. Use CLI to interact.\n";
    cout << "Note: edges' weights are treated as minutes for ETA calculations.\n";

    cli_loop(system);

    return 0;
}