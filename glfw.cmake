# this target guard exists so that if glfw was previously built by another
# project in the hierarchy it won't be redundantly built
if (TARGET glfw)
    message(STATUS "Found glfw")
else()
    message(STATUS "Installing glfw")

    include(FetchContent)
    FetchContent_Declare(glfw
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(glfw)
    if(NOT glfw_POPULATED)
        message(STATUS "Populating glfw")
        FetchContent_Populate(glfw)
        set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "shared")
        set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "glfw examples")
        set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "glfw tests")
        set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "glfw docs")
        set(GLFW_INSTALL ON CACHE INTERNAL "glfw install")
        set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "glfw vulkan") # "Assume the Vulkan loader is linked with the application"
        add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
    endif()

    FetchContent_Declare(gl3w
        GIT_REPOSITORY "https://github.com/skaslev/gl3w"
        GIT_SHALLOW ON)
    FetchContent_GetProperties(gl3w)
    if(NOT gl3w_POPULATED)
        message(STATUS "Populating gl3w")
        FetchContent_Populate(gl3w)
        add_subdirectory(${gl3w_SOURCE_DIR} ${gl3w_BINARY_DIR})
    endif()
endif()

