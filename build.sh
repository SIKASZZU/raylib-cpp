#!/bin/bash

echo 'BUILD STARTED...'

if g++ -std=c++20 main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main; then
    echo 'BUILD COMPLETE!'
    ./main
else
    echo 'BUILD FAILED!'
    exit 1
fi
