@echo off
cmake -B "build/" -S . -G Ninja -DDATA_PATH=C:\\Users\\Alex\\Documents\\metaforce-v0.1.3-173\\MP1
ninja -C "build/"
