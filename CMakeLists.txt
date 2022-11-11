cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)

project(nightingale VERSION 1.0)

add_executable(nightingale)
set_property(TARGET nightingale PROPERTY CXX_STANDARD 14)

# find_package(glfw3)
include(glfw.cmake)

# find_package(imgui)
include(imgui.cmake)

target_sources(nightingale
  PUBLIC
    main_glfw.cpp

    app.cpp
    audio.cpp
    editor.cpp
    widgets.cpp

    app.h
    audio.h
    editor.h
    widgets.h
    miniaudio.h
    embedded_font.inc

  PRIVATE
    libs/gl3w/GL/gl3w.c)

add_subdirectory("libs")
include_directories(
  ${PROJECT_SOURCE_DIR}/libs/nativefiledialog/src/include
)

add_library(imgui-node-editor STATIC)
set_property(TARGET imgui-node-editor PROPERTY CXX_STANDARD 14)
set(IMGUI_NODE_EDITOR_DIR libs/imgui-node-editor)
target_sources(imgui-node-editor
    PRIVATE
        ${IMGUI_NODE_EDITOR_DIR}/crude_json.cpp
#        ${IMGUI_NODE_EDITOR_DIR}/imgui_bezier_math.inl
        ${IMGUI_NODE_EDITOR_DIR}/imgui_canvas.cpp
#        ${IMGUI_NODE_EDITOR_DIR}/imgui_extra_math.inl
        ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor.cpp
        ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_api.cpp
        ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_internal.h
#        ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_internal.inl
    PUBLIC
        ${IMGUI_NODE_EDITOR_DIR}/crude_json.h
        ${IMGUI_NODE_EDITOR_DIR}/imgui_bezier_math.h
        ${IMGUI_NODE_EDITOR_DIR}/imgui_canvas.h
        ${IMGUI_NODE_EDITOR_DIR}/imgui_extra_math.h
        ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor.h
        )
target_include_directories(imgui-node-editor PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_NODE_EDITOR_DIR})

set_target_properties(nightingale PROPERTIES COMPILE_FLAGS "-DIMGUI_IMPL_OPENGL_LOADER_GL3W")

find_package(OpenGL REQUIRED)

target_include_directories(nightingale PUBLIC
    ${IMGUI_NODE_EDITOR_DIR}
    libs/gl3w)

target_compile_definitions(nightingale 
    PRIVATE BUILT_RESOURCE_PATH=${PROJECT_SOURCE_DIR})

target_link_libraries(nightingale
    imgui
    imgui-node-editor
    nativefiledialog
    glfw)

if (APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(nightingale ${COREFOUNDATION_LIBRARY})
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(nightingale ${COCOA_LIBRARY})
  find_library(OPENGL_LIBRARY OpenGL)
  target_link_libraries(nightingale ${OPENGL_LIBRARY})
endif (APPLE)

add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/embedded_font.inc"
    COMMAND binary_to_compressed_c -base85 "${PROJECT_SOURCE_DIR}/fonts/Shure Tech Mono Nerd Font Complete Mono.ttf" ShureTechMono > ${PROJECT_SOURCE_DIR}/embedded_font.inc
    COMMENT "Embedding font..."
    MAIN_DEPENDENCY "fonts/Shure Tech Mono Nerd Font Complete.ttf"
    VERBATIM)

install(TARGETS nightingale
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

