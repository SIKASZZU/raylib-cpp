#!/bin/bash

echo 'BUILD STARTED...'

if g++ -std=c++20 main.cpp src/level.cpp src/player.cpp src/camera_controller.cpp src/scene_renderer.cpp src/shader_system.cpp src/maze.cpp src/map.cpp \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main; then
    echo 'BUILD COMPLETE!'
    ./main
else
    echo 'BUILD FAILED!'
    exit 1
fi
