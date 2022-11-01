cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)

project(nightingale VERSION 1.0)

add_executable(nightingale)
set_property(TARGET nightingale PROPERTY CXX_STANDARD 14)

# add_subdirectory("imgui")

# find_package(glfw3)
include(glfw.cmake)

# find_package(imgui)
include(imgui.cmake)

target_sources(nightingale
  PUBLIC
    main_glfw.cpp
    app.cpp
    audio.cpp

    app.h
    audio.h
    miniaudio.h
    
    ${gl3w_BINARY_DIR}/src/gl3w.c
)

# add_subdirectory("libs")
# include_directories(
#   ${PROJECT_SOURCE_DIR}/libs/nativefiledialog/src/include
# )
set_target_properties(nightingale PROPERTIES COMPILE_FLAGS "-DIMGUI_IMPL_OPENGL_LOADER_GL3W")

find_package(OpenGL REQUIRED)
#target_link_libraries(imgui PUBLIC ${OPENGL_LIBRARIES})

#find_package(gl3w REQUIRED)
#include_directories(
#  ${gl3w_SOURCE_DIR}/include
#)

#find_package(GLEW REQUIRED)
#target_include_directories(nightingale PUBLIC ${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES})
#include_directories(
#    libs/gl3w
#)
include_directories(
  ${gl3w_HEADER_DIR}
)

target_compile_definitions(nightingale 
    PRIVATE BUILT_RESOURCE_PATH=${PROJECT_SOURCE_DIR})

target_link_libraries(nightingale
    imgui
    glfw
    gl3w
)

if (APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(nightingale ${COREFOUNDATION_LIBRARY})
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(nightingale ${COCOA_LIBRARY})
  find_library(OPENGL_LIBRARY OpenGL)
  target_link_libraries(nightingale ${OPENGL_LIBRARY})
endif (APPLE)

install(FILES
    "nightingale_rsrc/ShareTechMono-Regular.ttf"
    "nightingale_rsrc/fontawesome-webfont.ttf"
    DESTINATION bin/nightingale_rsrc)

install(TARGETS nightingale
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

