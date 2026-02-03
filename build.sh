#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

make

if [ -f "compile_commands.json" ]; then
  cp compile_commands.json ..
fi
