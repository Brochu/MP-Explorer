@echo off
cmake -B "build/" -S . -G Ninja
ninja -C "build/"