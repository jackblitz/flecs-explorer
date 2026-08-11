# flecs-explorer

A lightweight, high-performance terminal user interface (TUI) application written in C99 to inspect, debug, and visualize local [Flecs ECS](https://github.com/SanderMertens/flecs) engine states over the Flecs REST API.

## Features (v0.1.0 MVP)
- **Local REST Connection**: Auto-connects to `http://localhost:27750` via `libcurl`.
- **Multi-Pane TUI**: Responsive split layout built with `ncursesw` featuring focus switching and Vim key navigation.
- **Entity Hierarchy Tree**: Visual tree view rendering `ChildOf` entity relationships with expand/collapse and live filtering.
- **Component Inspector**: Real-time display of component struct data, pairs, and tags for highlighted entities.

## Documentation
- [North Star (Issue #1)](https://github.com/jackblitz/flecs-explorer/issues/1)
- [Technical Blueprint (Issue #2)](https://github.com/jackblitz/flecs-explorer/issues/2)
- [Release Plan v0.1.0 MVP (Issue #3)](https://github.com/jackblitz/flecs-explorer/issues/3)
