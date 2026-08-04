# RAPID RACING

A 2D top-down racing game built with C++ and SFML, featuring physics-based
car handling, car-to-car and wall collisions, and AI opponents that read the
track's geometry to brake, corner, and pass opponents.

## Features

- **Physics-driven driving** : acceleration, friction, angular velocity based turning,
  and impulse-based collision response for both cars and walls.
- **Curvature-aware AI** : analyzes the track's waypoints to detect
  corners, find apexes, and brake into turns at a speed scaled to how sharp
  each corner is. Also reacts to traffic: following, passing, and
  varying how aggressively they drive based on race position.
- **Three game modes:**
  - **Time Trial** : solo runs against your best lap time.
  - **PVP** : local two player split-input racing.
  - **VS AI** : race against up to 7 AI opponents, or watch the AI race using the spectator mode.
- **Persistent leaderboard** of best lap times, saved to disk as a .txt file.
- **Adjustable settings** : car performance preset, lap count, AI
  difficulty/count, mute, and a debug overlay toggle.
- **Debug display** which details AI's decision making process via renders.
- **Multi-track compatibility** : Graphics displays made to help support future track additions, AI and waypoint handler already scales for future tracks.

## Controls

| Action | Player 1 | Player 2 (PVP) |
| --- | --- | --- |
| Steer | `A` / `D` or arrow keys | `A` / `D` |
| Accelerate / Brake | `W` / `S` or arrow keys | `W` / `S` |
| Reset if stuck | `R` | `M` |
| Spectator: switch car | `Tab` (while spectating) | — |
| Spectator: exit | `Esc` (while spectating) | — |

## Project structure

| File | Responsibility |
| --- | --- |
| `Game` | Main loop, state transitions, input, race logic |
| `Car` / `Player` / `AI` | Vehicle physics and per-frame movement |
| `AIController` | AI decision-making (steering, speed, overtaking) |
| `WaypointHandler` | Track curvature analysis, corner/brake-zone data |
| `CollisionHandler` | Car-wall and car-car collision detection/response |
| `Track` | Track image, waypoints, road-surface detection |
| `TrackLoader` | Loads track data by parsing a JSON file |
| `Graphics` | All rendering — HUD, menus, minimap, gameplay |
| `StateManager` | Screen/state stack and audio transitions |
| `Leaderboard` | Best-lap persistence |

## Requirements

- C++17 or later
- [SFML](https://www.sfml-dev.org/) (Graphics + Audio modules)
- [Catch2](https://github.com/catchorg/Catch2) (fetched automatically via CMake for the test suite)

## Building

Link against SFML's `graphics`, `audio`, `window`, and `system` modules, and
compile the sources in this repo. Track and car assets are expected under
`assets/` (see the paths used in `Game::init()`).

## Testing

The project has an automated Catch2 test suite (`unit_tests` target) covering:

- **`WaypointHandler`** — curvature computation, smoothing, straight/corner
  classification, corner-zone detection, brake-zone generation, and
  scale-factor consistency across differently-sized track images.
- **`AIController`** — construction/reset, aggro bounds, speed decisions
  (in-corner and on-straight), steering sanity, single-tick state
  transitions, and traffic-corridor detection (car ahead/behind/beside,
  projection-distance cutoff).
- **`CollisionHandler`** — wall detection (road/wall/tolerance/bounds), SAT
  car-car overlap detection, wall-collision response physics (head-on,
  rear, side, corner, deep-penetration), and car-car impulse/overlap
  resolution, including invincibility and inactive-car filtering.

Track and car geometry for tests are built synthetically (in-memory images
and generated waypoint arrays) rather than loading real assets, so the
suite runs headlessly without needing track/car image files on disk.

Build and run with:
cmake --build build --target unit_tests
ctest --test-dir build --output-on-failure
**Not yet covered explicitly using ctest:** multi-tick AI state-machine behavior (`Following` →
`Passing` and its abort conditions), and `Game`/`Car` orchestration logic —
these are currently validated through manual playtesting.

## Known limitations

- Track-loading is JSON-driven (`TrackLoader`), but adding a new track still
  requires manually producing that track's left/right waypoint coordinates
  by hand — there's no waypoint-extraction tooling yet.
- Track images should be a centered crop of the track itself (track fills the
  frame, minimal padding) at any resolution, as `scaleFactor` is derived from
  the image's pixel diagonal, which several distance constants (braking distance, smoothing radius) scale against.

## Extendability

Adding a new track is straightforward and requires minimal changes to the source code:

1. **Prepare assets**  
   - A **track image** (tightly cropped to the racing surface).  
   - A **minimap image** (thumbnail for the track selection screen).

2. **Write a JSON definition**  
   Create a JSON file with the following fields (see `TrackLoader.cpp` for the full schema):

   - `"image"` – path to the track texture.  
   - `"minimapImage"` – path to the minimap texture.  
   - `"waypoints"` – an array of `[x1, y1, x2, y2]` arrays, where `(x1, y1)` is the left boundary and `(x2, y2)` is the right boundary of each waypoint.  
   - `"startPosA"`, `"startPosB1"`, `"startPosB2"` – grid start positions (Can be horizontal or vertical, just not diagonal.)  
   - `"startRowSpacing"`, `"startAngle"`, `"minimapScale"`, `"id"` – starting grid layout and track metadata.

3. **Register the track**  
   Append the JSON file path to the `trackPaths` vector inside `Game::init()`.

Once registered, the track appears in the selection menu automatically. The AI, waypoint handler, and physics all scale dynamically using the track’s `scaleFactor`, so no additional tuning is required for different resolutions.

## Credits

- CMake, SFML, Catch2, Dependencies for release file extraction
- All assets are open source or produced originally with AI assistance (cars modelled after Lamborghini Veneno)
