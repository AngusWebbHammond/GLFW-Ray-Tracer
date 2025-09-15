cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(RayTracer)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(AddGLFW)
include(AddGLM)

include_directories(
    deps/glad/include
    deps/imgui
    deps/imgui/backends
)

set(GLAD_GL "deps/glad/src/gl.c")

set(IMGUI_SOURCES
    deps/imgui/imgui.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
)

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

set (APPLICATION_SOURCES
    src/Core/application.cpp
    src/Core/application.h
    src/Core/ui.cpp
    src/Core/ui.h
    src/Renderer/renderer.cpp
    src/Renderer/renderer.h
    src/Renderer/rayTracer.cpp
    src/Renderer/rayTracer.h
    src/Shader/shader.h 
    src/Shader/shader.cpp
)

add_executable(main src/main.cpp ${GLAD_GL} ${IMGUI_SOURCES} ${APPLICATION_SOURCES})

target_link_libraries(main PRIVATE glfw glm)

add_compile_definitions(PROJECT_DIR="${CMAKE_SOURCE_DIR}")

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set '3D-RENDERER' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main )
    endif()
endif()
