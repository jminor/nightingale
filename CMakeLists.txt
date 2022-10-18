cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)

project(nightingale VERSION 1.0)

add_subdirectory("imgui")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(nightingale)

target_sources(nightingale
  PUBLIC
    main_glfw.cpp
    app.cpp
    audio.cpp
    widgets.cpp
)

target_link_directories(nightingale
  PUBLIC
    /opt/local/lib
    /opt/homebrew/lib
    /usr/local/lib
)

target_link_libraries(nightingale
  IMGUI
  glfw
)

file(COPY fonts/ShareTechMono-Regular.ttf DESTINATION fonts)
file(COPY fonts/fontawesome-webfont.ttf DESTINATION fonts)

if (APPLE)
  find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
  find_library(COREAUDIO_LIBRARY CoreAudio)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(nightingale
    ${AUDIOTOOLBOX_LIBRARY}
    ${COREAUDIO_LIBRARY}
    ${COREFOUNDATION_LIBRARY}
  )
endif (APPLE)

