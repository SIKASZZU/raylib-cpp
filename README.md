# raylib-cpp

## Build & run

```sh
./build.sh
```
or
```sh
g++ -std=c++20 main.cpp src/level.cpp src/player.cpp src/camera_controller.cpp src/scene_renderer.cpp src/shader_system.cpp \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main
```


## Run
Run from the repository root so model and shader resource paths resolve:

```sh
./main
```

## Debug run

If program closes without specific code or line error run:
```sh
gdb -batch -ex run -ex bt ./main
```

## Source layout

- `main.cpp`: application setup and main loop
- `src/level.*`: wall instances shared by rendering, shadows, and collision
- `src/player.*`: player physics and wall-instance collision
- `src/camera_controller.*`: first-person camera and look controls
- `src/scene_renderer.*`: visible scene drawing and frustum culling
- `src/shader_system.*`: lighting shader and shadow-map setup
- `resources/shaders/`: GLSL shader sources

Map dimensions and tile spacing live with the map implementation in
`src/map.hpp`. Wall collision and culling extents are grouped with
`WallInstance` in `src/level.hpp`; despite the old configuration name,
`WallGeometry::Height` is used for player collision and renderer culling, not
to scale the rendered mesh. The render scale is defined locally in
`src/scene_renderer.cpp`. Walls are stored as `WallInstance` values in
`Level::walls`; their position and rotation are shared by rendering and
collision.