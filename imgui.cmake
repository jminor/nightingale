# this target guard exists so that if imgui was previously built by another
# project in the hierarchy it won't be redundantly built
if (TARGET imgui)
    message(STATUS "Found imgui")
else()
    message(STATUS "Installing imgui")

    include(FetchContent)

    FetchContent_Declare(imgui
        GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
        #GIT_TAG "master"
        GIT_TAG "docking"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR ${imgui_SOURCE_DIR})

#    FetchContent_Declare(imgui_with_addons
#        GIT_REPOSITORY "https://github.com/Flix01/imgui.git"
#        GIT_TAG "imgui_with_addons"
#        GIT_SHALLOW ON)
#    FetchContent_MakeAvailable(imgui_with_addons)
#    set(IMGUI_ADDONS_DIR ${imgui_with_addons_SOURCE_DIR})
#
#    FetchContent_Declare(implot
#        GIT_REPOSITORY "https://github.com/epezent/implot.git"
#        #GIT_TAG "master"
#        GIT_SHALLOW ON)
#    FetchContent_MakeAvailable(implot)
#    set(IMPLOT_DIR ${implot_SOURCE_DIR})

    add_library(imgui STATIC)
    
    target_sources(imgui
    PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.h
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.h

        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imstb_rectpack.h
        ${IMGUI_DIR}/imstb_textedit.h
        ${IMGUI_DIR}/imstb_truetype.h

#        ${IMGUI_ADDONS_DIR}/addons/imguifilesystem/imguifilesystem.cpp
#        ${IMGUI_ADDONS_DIR}/addons/imguifilesystem/imguifilesystem.h
#
#        ${IMGUI_ADDONS_DIR}/addons/imguihelper/imguihelper.cpp
#        ${IMGUI_ADDONS_DIR}/addons/imguihelper/imguihelper.h

#        ${IMPLOT_DIR}/implot.cpp
#        ${IMPLOT_DIR}/implot.h
#
#        ${IMGUI_DIR}/imgui_stdlib.cpp
#        ${IMGUI_DIR}/imgui_stdlib.h
    )

    set_property(TARGET imgui PROPERTY CXX_STANDARD 14)
	set_target_properties(imgui PROPERTIES COMPILE_FLAGS "-DIMGUI_IMPL_OPENGL_LOADER_GL3W")

    target_include_directories(imgui
    PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${IMGUI_DIR}/addons/imguifilesystem
        ${IMGUI_DIR}/addons/imguihelper
        ${glfw_SOURCE_DIR}/include
    )

#    find_package(OpenGL REQUIRED)
#    target_link_libraries(imgui PUBLIC ${OPENGL_LIBRARIES})

#    set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui)

#    FetchContent_GetProperties(imgui)
#    if(NOT imgui_POPULATED)
#        FetchContent_Populate(imgui)
#        set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "shared")
#        set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "imgui examples")
#        set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "imgui tests")
#        set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "imgui docs")
#        set(GLFW_INSTALL ON CACHE INTERNAL "imgui install")
#        set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "imgui vulkan") # "Assume the Vulkan loader is linked with the application"
#        add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})
#    endif()
endif()

