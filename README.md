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

## Source layout

- `main.cpp`: application setup and main loop
- `src/level.*`: wall instances shared by rendering, shadows, and collision
- `src/player.*`: player physics and wall-instance collision
- `src/camera_controller.*`: first-person camera and look controls
- `src/scene_renderer.*`: visible scene drawing and frustum culling
- `src/shader_system.*`: lighting shader and shadow-map setup
- `resources/shaders/`: GLSL shader sources

Walls are stored as mutable `WallInstance` values in `Level::walls`. Rendering,
shadow casting, and player collision all read the same position, rotation, and
scale. Update a wall instance's transform to move it; the shadow map is refreshed
each frame to reflect those changes.