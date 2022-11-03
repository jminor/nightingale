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

    add_executable(binary_to_compressed_c
        ${IMGUI_DIR}/misc/fonts/binary_to_compressed_c.cpp
    )
    set_property(TARGET binary_to_compressed_c PROPERTY CXX_STANDARD 14)

endif()

