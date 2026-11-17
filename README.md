# Smart Campus Navigation System

**PBL Project** · Tanishq Gola · Roll No. 25/SET/BCSE/519

A web version of the console-based C++ campus navigator. The website sends the selected start and destination to a **C++ REST API (Crow)**. The API runs **Dijkstra's Shortest Path Algorithm** on the original adjacency matrix and returns the route and distance. The website shows them on an interactive campus map.

The adjacency matrix, the 11 campus connections, their distances and the Dijkstra logic are unchanged from the console project. Only the `cin`/`cout` menu was replaced by REST endpoints.

---

## Project structure

```
smart-campus/
├── README.md
├── backend/
│   ├── main.cpp            <- C++ server: graph + Dijkstra + REST API
│   └── CMakeLists.txt      <- build file (downloads Crow + Asio automatically)
└── frontend/
    ├── index.html          <- dashboard page
    ├── css/style.css       <- blue & white theme, responsive layout
    └── js/app.js           <- map drawing + REST API calls
```

## REST API

| Method | Endpoint          | Description                                              |
|--------|-------------------|----------------------------------------------------------|
| GET    | `/api/health`     | Server status                                            |
| GET    | `/api/locations`  | The 8 locations and all 11 connections with distances    |
| POST   | `/api/route`      | Body `{"source":0,"destination":5}` (ids 0-7) returns route and distance |

Location ids follow your original array order:
`0 Main Gate · 1 Main Institute · 2 Library · 3 Canteen · 4 Sports Science Center · 5 Dental College · 6 M Block · 7 Bus Parking`

Example:

```bash
curl -X POST http://localhost:18080/api/route \
     -H "Content-Type: application/json" \
     -d '{"source":0,"destination":5}'
# -> Main Gate -> Main Institute -> Library -> Dental College, totalDistance 550
```

---

## 1. Install the requirements

You need a **C++17 compiler**, **CMake (3.15+)** and **Git**. Crow and Asio are free and are downloaded automatically during the first build, so you need an internet connection once. There is no API key.

### Linux (Ubuntu / Debian)
```bash
sudo apt update
sudo apt install -y g++ cmake make git
```

### macOS
```bash
xcode-select --install
brew install cmake git
```

### Windows (MSYS2 MinGW64)
1. Install MSYS2 from https://www.msys2.org
2. Open the **"MSYS2 MinGW 64-bit"** terminal and run:
```bash
pacman -Syu
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make git
```
> The backend was compiled and tested on Linux (Ubuntu, g++ 13). The steps for macOS and Windows follow the same CMake flow, but I could not test them here.

---

## 2. Compile the backend

From the `smart-campus` folder:

**Linux / macOS**
```bash
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

**Windows (MSYS2 MinGW64 terminal)**
```bash
cd backend
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

The executable is created at `backend/build/campus_server` (`campus_server.exe` on Windows).

## 3. Run the backend

```bash
# still inside the backend folder
./build/campus_server            # Windows: build\campus_server.exe
```

You should see:

```
Serving website from: .../smart-campus/frontend
  Smart Campus Navigation System backend is running
  Open the website:  http://localhost:18080
```

Keep this terminal open while you use the website.

## 4. Launch the website

**Option A (easiest):** the C++ server also serves the website. Open this in your browser:

### http://localhost:18080

**Option B:** serve the `frontend` folder separately (the API stays on port 18080, and CORS is already enabled):
```bash
cd frontend
python3 -m http.server 5500      # Windows: python -m http.server 5500
# open http://localhost:5500
```
Opening `frontend/index.html` directly by double-click also works while the backend is running.

> If the backend is not running, the site shows **"Backend offline"** and a message explaining how to start it.

---

## How to use the website

1. Pick a **Starting Location** and a **Destination**. You can use the dropdowns or click markers on the map: the first click sets the start and the second sets the destination.
2. Click **Find Shortest Route**.
3. See the highlighted route on the map, the step-by-step directions, and the total distance in metres.
4. Use the **⇄** button to swap start and destination, or **Reset** to clear everything.
5. Use the sidebar to open **Campus Locations** (all locations and their direct connections, with "Start here" and "Go here" buttons) and **About Project**.

## Troubleshooting

| Problem | Fix |
|---|---|
| `Cannot reach the C++ backend` on the page | Start `campus_server` and keep its terminal open, then refresh. |
| `Address already in use` | Another program is using port 18080. Close it, or change `.port(18080)` in `main.cpp` and `API_BASE` in `js/app.js`. |
| CMake cannot download Crow or Asio | Check your internet connection or firewall. Git must be installed. |
| Server starts but shows "Frontend folder not found" | Run it from the `backend` folder, or pass the path: `./build/campus_server /path/to/frontend` |

## Changing the campus graph

Edit `createCampus()` in `backend/main.cpp` (same style as your original code), then rebuild. The website reads connections and distances from the API. If you add a new location, also add its pixel position to `LAYOUT` in `frontend/js/app.js` so it can be drawn on the map.
