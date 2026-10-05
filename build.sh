#!/bin/bash

echo 'BUILD STARTED...'
g++ main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main
echo 'BUILD COMPLETE!'

./main