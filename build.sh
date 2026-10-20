#!/bin/bash

echo 'BUILD STARTED...'

# this is stupid and does not scale well
if g++ -std=c++20 main.cpp src/level.cpp src/player.cpp src/camera_controller.cpp src/scene_renderer.cpp src/model_assets.cpp src/maze.cpp src/map.cpp src/asterion.hpp \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main; then
    echo 'BUILD COMPLETE!'
    ./main
else
    echo 'BUILD FAILED!'
    exit 1
fi
