#!/bin/bash

echo 'BUILD STARTED...'
gcc main.cpp -lraylib -o main
echo 'BUILD COMPLETE!'

./main