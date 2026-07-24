# Top-Down Racer

A 2D top-down racing game built with C++ and SFML, featuring physics-based
car handling, car-to-car and wall collisions, and AI opponents that read the
track's geometry to brake, corner, and pass on their own.

## Features

- **Physics-driven driving** — acceleration, friction, drift-like turning,
  and impulse-based collision response for both cars and walls.
- **Curvature-aware AI** — opponents analyze the track's waypoints to detect
  corners, find apexes, and brake into turns at a speed scaled to how sharp
  each corner is. They also react to traffic: following, passing, and
  varying how aggressively they drive based on race position.
- **Three game modes:**
  - **Time Trial** — solo runs against your best lap time.
  - **PVP** — local two-player split-input racing.
  - **VS AI** — race against up to 7 AI opponents, with a spectator mode
    that kicks in once you finish.
- **Persistent leaderboard** of best lap times, saved to disk.
- **Adjustable settings** — car performance preset, lap count, AI
  difficulty/count, mute, and a debug overlay toggle.
- **Debug display** which details AI's decision making process via renders.

## Controls

| Action | Player 1 | Player 2 (PVP) |
|---|---|---|
| Steer | `A` / `D` or arrow keys | `A` / `D` |
| Accelerate / Brake | `W` / `S` or arrow keys | `W` / `S` |
| Reset if stuck | `R` | `M` |
| Spectator: switch car | `Tab` (while spectating) | — |
| Spectator: exit | `Esc` (while spectating) | — |

## Project structure

| File | Responsibility |
|---|---|
| `Game` | Main loop, state transitions, input, race logic |
| `Car` / `Player` / `AI` | Vehicle physics and per-frame movement |
| `AIController` | AI decision-making (steering, speed, overtaking) |
| `WaypointHandler` | Track curvature analysis, corner/brake-zone data |
| `CollisionHandler` | Car-wall and car-car collision detection/response |
| `Track` | Track image, waypoints, road-surface detection |
| `Graphics` | All rendering — HUD, menus, minimap, gameplay |
| `StateManager` | Screen/state stack and audio transitions |
| `Leaderboard` | Best-lap persistence |

## Requirements

- C++17 or later
- [SFML](https://www.sfml-dev.org/) (Graphics + Audio modules)

## Building

Link against SFML's `graphics`, `audio`, `window`, and `system` modules, and
compile the sources in this repo. Track and car assets are expected under
`assets/` (see the paths used in `Game::init()`).

## Known limitations

- Ships with a single hardcoded track — no track-loading system yet.
- To add any track in the future, currently it will need to hardcode left and right waypoints.
- No automated tests; behavior is validated through manual playtesting.
