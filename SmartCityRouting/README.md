# Smart City Bus Routing & Tracking System

A full-stack **Smart City Routing System** built with **C++**, **Flask/Python**, and a lightweight **HTML/JS frontend**. The system models a city's bus network using advanced **Data Structures & Algorithms**, enabling:

* Shortest-path routing (Dijkstra)
* Optimized spatial routing (A*)
* Minimum Spanning Tree visualization (Prim’s MST)
* Bus tracking, movement simulation & ETA estimation
* Trie-based prefix search for stops
* History logging of all bus movements

This project demonstrates how classical DSA principles can power real-world navigation systems.

---

##  Features

### **1. Graph-Based City Network**

* Stops represented as graph nodes
* Routes represented as weighted edges (travel time)
* Bidirectional connections supported
* Fast adjacency-list representation

### **2. Shortest Path Routing (Dijkstra)**

* Computes fastest route between two stops
* Returns total cost and full stop-by-stop path
* Uses min-heap priority queue for optimal performance

### **3. A* Search Algorithm**

* Enhanced shortest-path using heuristics
* Euclidean distance used as an admissible heuristic
* Optimized for spatial routing (GPS-like)

### **4. Minimum Spanning Tree (Prim’s MST)**

* Computes minimal network connecting all stops
* Helps visualizing core backbone network
* Useful for planning or cost analysis

### **5. Bus System Simulation**

* Add buses with predefined routes
* Move individual buses one step
* Move all active buses simultaneously
* Automatically updates bus-stop index
* Bus → stop and stop → bus mappings maintained

### **6. ETA Estimation**

* ETA between any two stops
* ETA for a bus to reach a target stop
* Internally uses Dijkstra for travel time computation

### **7. Trie-Based Stop Search**

* Auto-complete stops by prefix
* Fast O(P + K) search
* Smooth UI for selecting stops

### **8. Movement History Log**

* Queue-based log storage
* Records latest movements with timestamps
* Efficient FIFO behavior

### **9. Full Web Application**

* Frontend: HTML, CSS, JavaScript
* Backend: Python Flask server
* Core engine: C++ executable
* Real-time integration via subprocess calls

---

##  Data Structures Used

### **Graph (Adjacency List)**

* `vector<vector<pair<int, double>>>` for storing neighbors and weights
* Efficient for sparse city networks

### **Min-Heap (Priority Queue)**

Used in:

* Dijkstra’s shortest path (`O((V+E) log V)`)
* A* search

### **Trie**

* Supports fast prefix search for stop suggestions
* O(P + K) lookup time

### **Hash Maps**

* `unordered_map<string, StopID>` for stop lookups
* `unordered_map<StopID, vector<string>>` for buses-at-stop

### **Vectors**

* Store routes of buses
* Store paths reconstructed from Dijkstra/A*

### **Parent Array / Predecessor Array**

* Used to reconstruct shortest paths

### **Queue**

* Used for movement history (FIFO)
* Bounded by max history size

### **Coordinate Storage**

* For A* heuristic (x, y positions)

---

##  Algorithms Implemented

### **Dijkstra’s Algorithm**

* Shortest path
* Used for ETA, routing
* Min-heap optimized

### **A* Search**

* Heuristic-based pathfinding
* Uses Euclidean distance

### **Prim’s Minimum Spanning Tree (MST)**

* Computes minimal connecting network
* Visualized in UI

### **Path Reconstruction**

* Using predecessor array
* Reverse traversal from target to source

### **Bus Movement Logic**

* Index-based route advancement
* Circular route traversal supported

---

##  System Architecture

```
Frontend (HTML/JS) → Flask API → C++ Engine → Results → UI
```

### Frontend

* Sends API requests
* Displays graph, routes, MST, logs
* Auto-complete via Trie

### Backend (Flask)

* Routes requests
* Executes C++ executable
* Parses and returns results

### C++ Engine

* Implements all DS + Algorithms
* Handles routing, MST, ETA, bus simulation

---

##  Installation

```
# Create and activate virtual environment
python3 -m venv venv
source venv/bin/activate     # macOS/Linux
venv\Scripts\activate       # Windows

# Install dependencies
pip install -r requirements.txt

# Run Flask server
python server.py
```

Then open:
`http://localhost:5000`

---

##  How to Use

### **1. View Stops & Buses**

* "List Stops" shows all available stops
* "List Buses" shows buses and their current positions

### **2. Routing (Dijkstra / A*)**

Enter start & end stops → click *Find*

* Shows shortest path + cost
* Graph visualization updates

### **3. MST**

Click *Show MST* → minimal connecting network displayed

### **4. ETA**

* ETA between two stops
* ETA for bus to reach a stop

### **5. Bus Controls**

* Move single bus
* Move all buses
* View movement history

### **6. Stop Suggestions (Trie)**

Type prefix → suggested stops appear instantly

---

##  Time Complexities

| Operation              | Complexity         |
| ---------------------- | ------------------ |
| Trie Search            | **O(P + K)**       |
| Stop Lookup (Hash Map) | **O(1)**       |
| Buses at a Stop        | **O(1 + B)**       |
| Dijkstra               | **O((V+E) log V)** |
| A*                     | **O((V+E) log V)** |
| MST (Prim)             | **O((V+E) log V)** |
| History Retrieval      | **O(H)**           |

---

##  Demo Features

* Dijkstra routing
* A* routing
* MST visualization
* Trie stop search
* ETA calculations
* Bus movement simulation
* Movement history
* Frontend + backend integration

---

##  Team Contributions

* **Aakarsh**: Graph, Dijkstra, A*, MST, Bus simulation, frontend, UI, ReadMe
* **Aabha**: Dijkstra Algo, Presentation
* **Kushagra**: Reconstruct Path
* **Mayank**: Prim's ALgo, CLI Interface, Demo dataset builder, main application loop, app.js, styles.css

---

##  License

This project is provided for academic purposes under the course guidelines.

---

##  Acknowledgments

* DSA Course Faculty
* Team Members
* Open-source C++ & Flask libraries
