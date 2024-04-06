cmake_minimum_required (VERSION 3.10)

project (MP-Explorer)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 20)

set (DEBUG_OPTIONS /Zi /Od)
set (RELEASE_OPTIONS /O2)

set (headers
    ${CMAKE_CURRENT_LIST_DIR}/include/d3dx12.h
    ${CMAKE_CURRENT_LIST_DIR}/include/camera.h
    ${CMAKE_CURRENT_LIST_DIR}/include/cmdallocpool.h
    ${CMAKE_CURRENT_LIST_DIR}/include/cmdmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/include/debugui.h
    ${CMAKE_CURRENT_LIST_DIR}/include/graphics.h
    ${CMAKE_CURRENT_LIST_DIR}/include/rootsignature.h
    ${CMAKE_CURRENT_LIST_DIR}/include/utility.h
    ${CMAKE_CURRENT_LIST_DIR}/include/world.h
)
set (imgui_headers
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imgui.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imgui_impl_dx12.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imgui_impl_sdl.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imgui_internal.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imgui/imstb_truetype.h
)

set (source
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/cmdallocpool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/cmdmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/debugui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/graphics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/rootsignature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/utility.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/world.cpp
)
set (imgui_source
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_impl_dx12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_impl_sdl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/imgui/imgui_widgets.cpp
)
set (tracy_source
    ${CMAKE_CURRENT_LIST_DIR}/tracy/TracyClient.cpp
)

set (libs
    d3d12.lib
    d3dcompiler.lib
    dxgi.lib
    dxguid.lib
    dxcompiler.lib

    assimp-vc143-mt.lib
    SDL2main
    SDL2
)

add_executable (MP-Explorer
    ${source} ${imgui_source}
    ${headers} ${imgui_headers}
    ${tracy_source}
)

target_compile_options(MP-Explorer PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(MP-Explorer PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")

target_include_directories(MP-Explorer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/include/dxc
        ${CMAKE_CURRENT_LIST_DIR}/include/imgui
        ${CMAKE_CURRENT_LIST_DIR}/include/SDL2
        ${CMAKE_CURRENT_LIST_DIR}/tracy/tracy #??? don't like that
)
target_link_directories(MP-Explorer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/libs
)
target_link_libraries(MP-Explorer ${libs})

# Tracy setup
add_compile_definitions(TRACY_ENABLE=ON)

set_target_properties(MP-Explorer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY       "${CMAKE_CURRENT_LIST_DIR}/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_LIST_DIR}/"
)

target_compile_definitions(MP-Explorer
    PRIVATE
    PATH="${DATA_PATH}"
)
