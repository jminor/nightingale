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
    widgets.cpp

    app.h
    audio.h
    widgets.h
    miniaudio.h
    embedded_font_ShareTechMono.inc
    embedded_font_fontawesome.inc

  PRIVATE
    libs/gl3w/GL/gl3w.c)

# add_subdirectory("libs")
# include_directories(
#   ${PROJECT_SOURCE_DIR}/libs/nativefiledialog/src/include
# )
set_target_properties(nightingale PROPERTIES COMPILE_FLAGS "-DIMGUI_IMPL_OPENGL_LOADER_GL3W")

find_package(OpenGL REQUIRED)

include_directories(
    libs/gl3w)

target_compile_definitions(nightingale 
    PRIVATE BUILT_RESOURCE_PATH=${PROJECT_SOURCE_DIR})

target_link_libraries(nightingale
    imgui
    glfw)

if (APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(nightingale ${COREFOUNDATION_LIBRARY})
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(nightingale ${COCOA_LIBRARY})
  find_library(OPENGL_LIBRARY OpenGL)
  target_link_libraries(nightingale ${OPENGL_LIBRARY})
endif (APPLE)

#set_source_files_properties(embedded_font_ShareTechMono.inc PROPERTIES GENERATED 1)
#set_source_files_properties(embedded_font_fontawesome.inc PROPERTIES GENERATED 1)

add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/embedded_font_ShareTechMono.inc"
    COMMAND binary_to_compressed_c -base85 ${PROJECT_SOURCE_DIR}/fonts/ShareTechMono-Regular.ttf ShareTechMono > ${PROJECT_SOURCE_DIR}/embedded_font_ShareTechMono.inc
    COMMENT "Embedding font: ShareTechMono-Regular.ttf"
    MAIN_DEPENDENCY "fonts/ShareTechMono-Regular.ttf"
    VERBATIM)

add_custom_command(
    OUTPUT "${PROJECT_SOURCE_DIR}/embedded_font_fontawesome.inc"
    COMMAND binary_to_compressed_c -base85 ${PROJECT_SOURCE_DIR}/fonts/fontawesome-webfont.ttf fontawesome > ${PROJECT_SOURCE_DIR}/embedded_font_fontawesome.inc
    COMMENT "Embedding font: fontawesome-webfont.ttf"
    MAIN_DEPENDENCY "fonts/fontawesome-webfont.ttf"
    VERBATIM)

install(TARGETS nightingale
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

