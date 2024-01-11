# MP-Explorer
Best try to write a renderer that is easy to work with. Should try and make it work with as simple C++ as possible (C w/ classes). Focus on using std::span in order to handle array of elements in algorithms.

##Tools Included

- CMake: Build System
- SDL2: Window Management
- ImGui: Immediate Mode Debug UI
- Tracy: Profiling

##Updated DirectX libraries/includes

With this project I would also like to understand how to manage different versions of the DirectX12 libraries better. I want to use a version of Dx12 Agility SDK locally setup in the project folder in order to have access to newer features (setup D3D12Core.dll?). I would also like to setup a good shader build pipeline.

- AgilitySDK: Look up how to setup D3D12Core.dll for a given project. Look up how to verify the setup was successful.
- DXC: I have two different options for the shader compilation
    - Either I still compile shaders at runtime, but with dxcompiler.dll/.lib with dxcapi.h included
    - Or I setup a pre-build shaders step in my CMakeLists.txt file but stil using a recent version of DXC.exe

My goal would be to be able to use newer features of Dx12 as well as compile my shaders with shadermodel >= 6.0

##Reverse Engineer MP Files

Info I can find to be documented here
