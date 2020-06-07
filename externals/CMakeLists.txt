# nbtp
option(NBTP_BUILD_PYTHON_MODULE OFF)
add_subdirectory(nbtp EXCLUDE_FROM_ALL)
target_include_directories(nbtp INTERFACE ./nbtp/include)

if (BUILD_GMINEMAP)
    # use GLVND for OpenGL
    cmake_policy(SET CMP0072 NEW)
    # use absolute path for imported libraries
    cmake_policy(SET CMP0076 NEW)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    # imgui
    add_library(imgui INTERFACE)
    target_include_directories(imgui INTERFACE
        ./imgui
        ./imgui/examples/
        ./imgui/examples/libs/gl3w/
    )
    target_sources(imgui INTERFACE
        ./imgui/imgui_widgets.cpp
        ./imgui/imgui_draw.cpp
        ./imgui/imgui.cpp
        ./imgui/imgui.h
        ./imgui/examples/libs/gl3w/GL/gl3w.c
        ./imgui/examples/libs/gl3w/GL/gl3w.h
        ./imgui/examples/imgui_impl_glfw.cpp
        ./imgui/examples/imgui_impl_glfw.h
        ./imgui/examples/imgui_impl_opengl3.cpp
        ./imgui/examples/imgui_impl_opengl3.h
    )
    target_compile_definitions(imgui INTERFACE -DIMGUI_IMPL_OPENGL_LOADER_GL3W)
    target_link_libraries(imgui INTERFACE glfw OpenGL::GL)
    # tinyfiledialogs
    add_library(tinyfiledialogs INTERFACE)
    target_include_directories(tinyfiledialogs INTERFACE ./tinyfiledialogs)
    target_sources(tinyfiledialogs INTERFACE
        ./tinyfiledialogs/tinyfiledialogs.c
        ./tinyfiledialogs/tinyfiledialogs.h
    )
endif()
